#include <openssl/crypto.h>
#if defined(WIN32)
#include <windows.h>
#else
#include <pthread.h>
#endif

#if defined(WIN32)
    #define MUTEX_TYPE HANDLE
    #define MUTEX_SETUP(x) (x) = CreateMutex(NULL, FALSE, NULL)
    #define MUTEX_CLEANUP(x) CloseHandle(x)
    #define MUTEX_LOCK(x) WaitForSingleObject((x), INFINITE)
    #define MUTEX_UNLOCK(x) ReleaseMutex(x)
    #define THREAD_ID GetCurrentThreadId()
#else
    #define MUTEX_TYPE pthread_mutex_t
    #define MUTEX_SETUP(x) pthread_mutex_init(&(x), NULL)
    #define MUTEX_CLEANUP(x) pthread_mutex_destroy(&(x))
    #define MUTEX_LOCK(x) pthread_mutex_lock(&(x))
    #define MUTEX_UNLOCK(x) pthread_mutex_unlock(&(x))
    #define THREAD_ID pthread_self()
#endif

static MUTEX_TYPE * mutex_buf = NULL;

static void lock_cb(int mode, int n, const char * file, int line)
{
	if (mode & CRYPTO_LOCK)
	{
		MUTEX_LOCK(mutex_buf[n]);
	}
	else
	{
		MUTEX_UNLOCK(mutex_buf[n]);
	}
}

static unsigned long id_cb(void)
{
	return ((unsigned long)THREAD_ID);
}

int multi_thread_setup(void)
{
	mutex_buf = (MUTEX_TYPE *) malloc(CRYPTO_num_locks( ) * sizeof(MUTEX_TYPE));
	if(!mutex_buf)
		return -1;
	for (int i = 0; i < CRYPTO_num_locks( ); i++)
		MUTEX_SETUP(mutex_buf[i]);
	CRYPTO_set_id_callback(id_cb);
	CRYPTO_set_locking_callback(lock_cb);
	return 0;
}

void multi_thread_cleanup(void)
{
	if (!mutex_buf)
	{
		return;
	}
	CRYPTO_set_id_callback(NULL);
	CRYPTO_set_locking_callback(NULL);
	for (int i = 0; i < CRYPTO_num_locks( ); i++)
	{
		MUTEX_CLEANUP(mutex_buf[i]);
	}
	free(mutex_buf);
	mutex_buf = NULL;
}

