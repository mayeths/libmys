//////// The following idea did not applied because I'm exhausted. Have fun with it.

// The annoying part of STREAM is the bandwidth of one core on morden machine
// can varies from 1x to 10x according to you parallelism configuation.
// To acquire a picture of bandwidth over parallelism, a fixed size configuation may
// lead to either the sequential run becomes too slow (with a appropriate array size for 128 cores run),
// or the full-parallelism run becomes too fast (with the size for 1 core run).
// How can we configure the size painlessly to save our life?
// Someday I noticed that we can use log() funtions to slow down the change of array size,
// resulting in only 1x to 3x variant in time between these two extreme conditions.
// Even if the number of cores increase dramatically, log2() or log10() will sail through it.
// So, I got some conditions here:
// - Original STREAM suggested tu run with 4 times of the size of LLC
// - Array with ~100MB size is a good taste on morden 3GHz machine
//   (it will cost 1ms ~ 1000ms to run in my rule of thumb)
// We use 128MB / 4 = 32MB as a threshold for STREAM. When user
// provided size is less than 32MB, it may be too fast for the timer.
#define SIZE_THRESHOLD "32MB"
// using local_array_size = min_size * (log2(ncores)+1) / (log2(nranks)+1) on a 8 cores machine,
// the rank0 local_array_size ranges from 4 (sequential) to 1 (full-parallelism) times of min_size.

/* Table of multiplier = (log2(ncores)+alpha) / (log2(nranks)+alpha)
   +--------+-----------+----------+--------+--------+--------+
   | ncores | log2+0.25 | log2+0.5 | log2+1 | log2+2 | log2+3 |
   +--------+-----------+----------+--------+--------+--------+
   |      1 |       1.0 |      1.0 |    1.0 |    1.0 |    1.0 |
   |      2 |*      5.0 |      3.0 |    2.0 |    1.5 |    1.3 |
   |      4 |       9.0 |*     5.0 |    3.0 |    2.0 |    1.7 |
   |      8 |      13.0 |*     7.0 |*   4.0 |    2.5 |    2.0 |
   |     12 |      15.3 |      8.2 |*   4.6 |    2.8 |    2.2 |
   |     16 |      17.0 |      9.0 |*   5.0 |    3.0 |    2.3 |
   |     20 |      18.3 |      9.6 |*   5.3 |    3.2 |    2.4 |
   |     24 |      19.3 |     10.2 |*   5.6 |    3.3 |    2.5 |
   |     32 |      21.0 |     11.0 |*   6.0 |    3.5 |    2.7 |
   |     48 |      23.3 |     12.2 |*   6.6 |    3.8 |    2.9 |
   |     56 |      24.2 |     12.6 |*   6.8 |    3.9 |    2.9 |
   |     64 |      25.0 |     13.0 |    7.0 |*   4.0 |    3.0 |
   |     72 |      25.7 |     13.3 |    7.2 |*   4.1 |    3.1 |
   |     80 |      26.3 |     13.6 |    7.3 |*   4.2 |    3.1 |
   |     88 |      26.8 |     13.9 |    7.5 |*   4.2 |    3.2 |
   |     96 |      27.3 |     14.2 |    7.6 |*   4.3 |    3.2 |
   |    128 |      29.0 |     15.0 |    8.0 |*   4.5 |    3.3 |
   |    192 |      31.3 |     16.2 |    8.6 |*   4.8 |    3.5 |
   |    240 |      32.6 |     16.8 |    8.9 |*   5.0 |    3.6 |
   |    256 |      33.0 |     17.0 |    9.0 |*   5.0 |    3.7 |
   |    512 |      37.0 |     19.0 |   10.0 |*   5.5 |*   4.0 |
   |   1024 |      41.0 |     21.0 |   11.0 |    6.0 |*   4.3 |
   |   2048 |      45.0 |     23.0 |   12.0 |    6.5 |*   4.7 |
   |   4096 |      49.0 |     25.0 |   13.0 |    7.0 |*   5.0 |
   |   8192 |      53.0 |     27.0 |   14.0 |    7.5 |*   5.3 |
   +--------+-----------+----------+--------+--------+--------+
                   (rank0 local_array_size in sequential run)
   Multiplier = ------------------------------------------------
                (rank0 local_array_size in full-parallelism run)
*/

// So instead of using alpha = 1 all the way down which introduce 8.0
// multiplies on 128 cores, switch alpha by ncores is a better practice
// for sure. Then, the code should be:

typedef unsigned long long int size_t;

double log2(double x);
size_t parse_readable_size(const char *);

size_t get_local_array_size(size_t LLC_size, int ncores, int nranks)
{
   struct condition_t {
      int ncores;
      double alpha;
   };
   static const condition_t conditions[] = {
      {.ncores = 2,   .alpha = 0.25},
      {.ncores = 4,   .alpha = 0.50},
      {.ncores = 8,   .alpha = 1.00},
      {.ncores = 64,  .alpha = 2.00},
      {.ncores = 512, .alpha = 3.00},
   };
   double alpha = 4; // for ncores == 1, 4 times of LLC_size
   if (ncores > 1) {
      for (int i = 0; i < sizeof(conditions) / sizeof(condition_t); i++) {
         if (ncores < conditions[i].ncores)
            break;
         alpha = conditions[i].alpha;
      }
   }

   size_t threshold = parse_readable_size(SIZE_THRESHOLD);
   // full-parallelism
   size_t fp_local_array_size = threshold > LLC_size ? threshold : LLC_size;
   double multiplier = (log2(ncores) + alpha) / (log2(nranks) + alpha);
   return fp_local_array_size * multiplier;
}

// We can also calculate every BEST alpha for current ncores and nranks,
// which has output always be 4 * fp_local_array_size.
