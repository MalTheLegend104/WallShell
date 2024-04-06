# Custom Threads

> You must define `CUSTOM_THREADS` if you wish to do this.

If you are in a freestanding environment, or you wish to use your own wrapper on an already supported system,
you must define the following functions:

```c
typedef /* Mutex type */ ws_mutex_t;
typedef /* ThreadID Type */ ws_thread_id_t;

void ws_lockMutex(ws_mutex_t* mut);
void ws_unlockMutex(ws_mutex_t* mut);
ws_mutex_t* ws_createMutex();
void ws_destroyMutex(ws_mutex_t* mut);

ws_thread_id_t getThreadID();
void ws_printThreadID(FILE* stream);

void ws_sleep(size_t ms);
```

> The function declarations are already provided for you.
> You do have to retype the entire typedef for `ws_mutex_t` and `ws_thread_id_t`.

These are further explanations:

- `ws_mutex_t` is simply expected to be whatever handle your threads have.
- `ws_thread_id_t` should be a unique identifier for each thread.
  - Since you're the one implementing `getThreadID()`, this can be anything.
  - Most systems, like Windows, Linux, and macOS all use `unsigned long`
- `void ws_lockMutex(ws_mutex_t* mut);`
  - Expected to lock the provided mutex. Mutex is always a pointer to mutex object.
- `void ws_unlockMutex(ws_mutex_t* mut);`
  - Expected to unlock the provided mutex. Mutex is always a pointer to the mutex object.
- `ws_mutex_t* ws_createMutex();`
  - Creates a mutex. Should return a pointer to a mutex, and `NULL` if one couldn't be created.
- `void ws_destroyMutex(ws_mutex_t* mut);`
  - Destroys the provided mutex. Mutex should be set to `NULL`, and ideally should be locked before destruction.
    - Locking it before destruction ensures that anything currently using it finishes first.
- `ws_thread_id_t getThreadID();`
  - Should return the thread ID for the calling thread.
  - The thread ID can be anything, it's just expected to be unique for each thread.
- `void ws_printThreadID(FILE* stream);`
  - Expected to print the calling threads thread ID to the stream.
  - This is required, since `fprintf` requires a format, and not all systems have the same thread identifiers.
- `void ws_sleep(size_t ms);`
  - Sleep the calling thread for `ms` milliseconds.
