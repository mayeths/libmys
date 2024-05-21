#include "_private.h"
#include "../memory.h"

#if defined(OS_LINUX) && defined(MYS_ENABLE_SHM)
struct _mys_shm_G_t {
    bool inited;
    mys_mutex_t lock;
    int program_id;
    size_t counter;
};

static struct _mys_shm_G_t _mys_shm_G = {
    .inited = false,
    .lock = MYS_MUTEX_INITIALIZER,
    .program_id = 0,
    .counter = 0,
};

static void _mys_shm_G_init()
{
    if (_mys_shm_G.inited == true)
        return;
    mys_mutex_lock(&_mys_shm_G.lock);
    {
        if (mys_mpi_myrank() == 0)
            _mys_shm_G.program_id = getpid();
        _mys_MPI_Bcast(&_mys_shm_G.program_id, 1, _mys_MPI_INT, 0, _mys_MPI_COMM_WORLD);
    }
    _mys_shm_G.inited = true;
    mys_mutex_unlock(&_mys_shm_G.lock);
}

MYS_API mys_shm_t mys_alloc_shared_memory(int owner_rank, size_t size)
{
    _mys_shm_G_init();
    mys_mutex_lock(&_mys_shm_G.lock);
    mys_shm_t shm;
    snprintf(shm._name, sizeof(shm._name), "/mys_%d_%zu", _mys_shm_G.program_id, _mys_shm_G.counter);
    _mys_shm_G.counter += 1;
    if (mys_mpi_myrank() == owner_rank) {
        shm._fd = shm_open(shm._name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        shm._size = size;
        ftruncate(shm._fd, shm._size);
        shm.mem = mmap(NULL, shm._size, PROT_READ | PROT_WRITE, MAP_SHARED, shm._fd, 0);
        memset(shm.mem, 0, shm._size);
        mys_memory_smp_mb();
        _mys_MPI_Barrier(_mys_MPI_COMM_WORLD);
    } else {
        _mys_MPI_Barrier(_mys_MPI_COMM_WORLD);
        shm._fd = shm_open(shm._name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        shm.mem = mmap(NULL, shm._size, PROT_READ | PROT_WRITE, MAP_SHARED, shm._fd, 0);
    }
    mys_mutex_unlock(&_mys_shm_G.lock);
    return shm;
}

MYS_API void mys_free_shared_memory(mys_shm_t *shm)
{
    _mys_shm_G_init();
    mys_mutex_lock(&_mys_shm_G.lock);
    {
        munmap(shm->mem, shm->_size);
        close(shm->_fd);
        shm_unlink(shm->_name);
        shm->mem = NULL;
    }
    mys_mutex_unlock(&_mys_shm_G.lock);
}

#endif

MYS_API mys_bits_t mys_bits(const void *data, size_t size)
{
    mys_bits_t res;
    memset(&res, 0, sizeof(mys_bits_t));
    const uint8_t *bytes = (const uint8_t *)data;
    int count = 0;
    for (int i = size - 1; i >= 0; --i) { // begin from high bytes
        for (int j = 7; j >= 0; --j) { // begin from high bits
            unsigned int bit = (bytes[i] >> j) & 1;
            res.bits[count++] = bit ? '1' : '0';
        }
    }
    return res;
}

MYS_API void mys_cache_flush(size_t nbytes)
{
    char * volatile arr = (char *)malloc(nbytes * sizeof(char));
    memset(arr, 0, nbytes);
    for (size_t i = 1; i < nbytes; i++) {
        arr[i] = i | (arr[i - 1]);
    }
    volatile char result;
    result = arr[nbytes - 1];
    result = (char)(uint64_t)(&arr[(uint64_t)result]);
    mys_atomic_fence(MYS_ATOMIC_RELEASE);
    free(arr);
}
