/* Work in progress */
#include "headers.hpp"
#include "partition.h"

std::vector<int> Lp;
std::vector<int> Lj;
std::vector<double> Lv;
std::vector<int> Up;
std::vector<int> Uj;
std::vector<double> Uv;
std::vector<double> Dv;
std::vector<int> levelOfOriginal;
std::vector<int> levelp;
std::vector<int> levelj;
int nlevels;

#ifndef barrier
#if defined(__x86_64__)
#define barrier() __asm__ __volatile__("": : :"memory")
#define smp_mb() __asm__ __volatile__("lock; addl $0,-132(%%rsp)" ::: "memory", "cc")
#define smp_rmb() barrier()
#define smp_wmb() barrier()
#elif defined(__aarch64__)
#define barrier() __asm__ __volatile__("dmb" ::: "memory")
#define smp_mb()  __asm__ __volatile__("dmb ish" ::: "memory")
#define smp_wmb() __asm__ __volatile__("dmb ishst" ::: "memory")
#define smp_rmb() __asm__ __volatile__("dmb ishld" ::: "memory")
#else
#error No architecture detected.
#endif
#endif

void levelset_analyse(int *Ap, int *Aj, double *Av, int nrow, int ncol) {
    Lp.resize(nrow + 1, 0);
    Lj.resize(0);
    Lv.resize(0);
    Up.resize(nrow + 1, 0);
    Uj.resize(0);
    Uv.resize(0);

    Dv.resize(0);

    levelOfOriginal.resize(nrow, 0);
    levelp.resize(nrow + 1, 0);
    nlevels = 0;
    for (int i = 0; i < nrow; i++) {
        const int rowstart = Ap[i];
        const int rowend   = Ap[i+1];
        int plevel = -1;
        for (int jj = rowstart; jj < rowend; jj++) {
            int j = Aj[jj];
            double v = Av[jj];
            if (j < i) {
                Lp[i + 1] += 1;
                Lj.push_back(j);
                Lv.push_back(static_cast<double>(v));
                plevel = std::max(plevel, levelOfOriginal[j]);
            } else if (j == i) {
                Dv.push_back(static_cast<double>(v));
            } else {
                Up[i + 1] += 1;
                Uj.push_back(j);
                Uv.push_back(static_cast<double>(v));
            }
        }
        int level = plevel + 1;
        levelOfOriginal[i] = level;
        levelp[level + 1] += 1;
        nlevels = std::max(level + 1, nlevels);
    }
    for (int i = 1; i < nrow + 1; i++) {
        Lp[i] += Lp[i - 1];
        Up[i] += Up[i - 1];
        levelp[i] += levelp[i - 1];
    }

    ASSERT_EQ(levelp[nlevels], nrow);
    levelp.resize(nlevels + 1);
    levelj.resize(nrow);
    {
        std::vector<int> levelsizes(nlevels, 0);
        for (int i = 0; i < nrow; i++) {
            int level = levelOfOriginal[i];
            levelj[levelp[level] + levelsizes[level]] = i;
            levelsizes[level] += 1;
        }
    }

#define BID(threadID, level, nthreads, nlevels) ((threadID) * (nlevels) + (level))

    const int nthreads = omp_get_max_threads();
    const int nbunchs = nthreads * nlevels;
    std::vector<int> bunchp(nbunchs + 1, 0);
    std::vector<int> perm2original(nrow, 0); // a.k.a. bunchj
    std::vector<int> original2perm(nrow, 0);
    std::vector<int> threadOfPerm(nrow, 0);
    int maxRowPerThread = -1;
    for (int threadID = 0; threadID < nthreads; threadID++) {
        for (int l = 0; l < nlevels; l++) {
            const int levelstart = levelp[l];
            const int levelend   = levelp[l+1];
            int tstart, tend;
            partition1DSimple(levelstart, levelend, nthreads, threadID, &tstart, &tend);
            int bunchsize = tend - tstart;
            int bunchID = BID(threadID, l, nthreads, nlevels);
            bunchp[bunchID + 1] = bunchp[bunchID] + bunchsize;
            int count = 0;
            for (int jj = tstart; jj < tend; jj++) {
                int originalID = levelj[jj];
                int permID = bunchp[bunchID] + count;
                perm2original[permID] = originalID;
                original2perm[originalID] = permID;
                count += 1;
                threadOfPerm[permID] = threadID;
            }
        }
        const int rowOfThisThread = bunchp[(threadID + 1) * nlevels] - bunchp[threadID * nlevels];
        maxRowPerThread = std::max(rowOfThisThread, maxRowPerThread);
    }

    ASSERT_EQ(bunchp[bunchp.size() - 1], nrow);


#define INVALID -1
    std::vector<int> globalReferrerRecord(nthreads * nbunchs, INVALID);
    std::vector<int> globalRefp(nthreads * (nlevels + 1), 0);
    std::vector<int> globalRefj(nthreads * nbunchs , 0);
    std::vector<int> globalRefMaxLevel(nthreads * nlevels, 0);
    std::vector<int> globalRefjjRecord(nthreads * nthreads, 0);
    #pragma omp parallel
    {
        const int threadID = omp_get_thread_num();
        int *localReferrerRecord = &globalReferrerRecord[threadID * nbunchs];
        int *localRefp = &globalRefp[threadID * (nlevels + 1)];
        int *localRefj = &globalRefj[threadID * nbunchs];
        int *localRefMaxLevel = &globalRefMaxLevel[threadID * nlevels];
        int *localRefjjRecord = &globalRefjjRecord[threadID * nthreads];
        for (int l = 0; l < nlevels; l++) {
            localRefp[l + 1] = localRefp[l];
            std::fill(localRefjjRecord, localRefjjRecord + nthreads, INVALID);
            const int bunchID = BID(threadID, l, nthreads, nlevels);
            const int levelstart = levelp[l];
            const int levelend   = levelp[l+1];
            for (int permID = bunchp[bunchID]; permID < bunchp[bunchID + 1]; permID++) {
                const int originalID = perm2original[permID];
                for (int Ljj = Lp[originalID]; Ljj < Lp[originalID + 1]; Ljj++) {
                    const int refOriginalID = Lj[Ljj];
                    const int refPermID = original2perm[refOriginalID];
                    const int refLevel = levelOfOriginal[refOriginalID];
                    const int refThreadID = threadOfPerm[refPermID];
                    const int refBunchID = BID(refThreadID, refLevel, nthreads, nlevels);

                    bool intraThreadRef = refThreadID == threadID;
                    bool alreadyRef = localReferrerRecord[refBunchID] != INVALID;
                    if (intraThreadRef || alreadyRef) {
                        DEBUG(0, "t %d l %d b %d p %d o %d, ref t %d l %d b %d p %d o %d | reason %d %d bunch %d", threadID, l, bunchID, permID, originalID, refThreadID, refLevel, refBunchID, refPermID, refOriginalID, intraThreadRef, alreadyRef, localReferrerRecord[refBunchID]);
                        continue;
                    }
                    if (localRefjjRecord[refThreadID] == INVALID) {
                        DEBUG(0, "t %d l %d b %d p %d o %d, ref t %d l %d b %d p %d o %d | first", threadID, l, bunchID, permID, originalID, refThreadID, refLevel, refBunchID, refPermID, refOriginalID);
                        localReferrerRecord[refBunchID] = bunchID;
                        int &jj = localRefp[l + 1];
                        localRefj[jj] = refBunchID;
                        localRefjjRecord[refThreadID] = jj;
                        jj += 1;
                        localRefMaxLevel[l] = refLevel;
                    } else {
                        const int &jj = localRefjjRecord[refThreadID];
                        DEBUG(0, "t %d l %d b %d p %d o %d, ref t %d l %d b %d p %d o %d | inplace b %d", threadID, l, bunchID, permID, originalID, refThreadID, refLevel, refBunchID, refPermID, refOriginalID, localRefj[jj]);
                        localRefj[jj] = refBunchID;
                    }
                }
            }
            #pragma omp barrier
        }
    }
#undef INVALID

#undef BID
}


extern "C"
void sor_setup()
{
}

void apply(double *x, double *b) {

}
