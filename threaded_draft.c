#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define WALLSHELL_LOGGING

#ifndef CUSTOM_THREADS

#ifdef _WIN32
#include <Windows.h>
typedef CRITICAL_SECTION ws_mutex_t;
typedef DWORD ws_thread_id_t;

#else
#include <pthread.h>
typedef pthread_mutex_t ws_mutex_t;
typedef uint64_t ws_thread_id_t;
#endif // _WIN32

#endif

FILE* ws_out_stream;
ws_mutex_t* internal_mutex;

void ws_lockMutex(ws_mutex_t* mut);
void ws_unlockMutex(ws_mutex_t* mut);

ws_mutex_t* ws_createMutex();
void ws_destroyMutex(ws_mutex_t* mut);

ws_thread_id_t ws_getThreadID();
void ws_internal_printThreadID(FILE* stream);

void ws_sleep(size_t ms);

#ifndef CUSTOM_THREADS

#ifdef _WIN32
void ws_lockMutex(ws_mutex_t* mut) { EnterCriticalSection(mut); }
void ws_unlockMutex(ws_mutex_t* mut) { LeaveCriticalSection(mut); }

ws_mutex_t* ws_createMutex() {
	ws_mutex_t* mut = (ws_mutex_t*) malloc(sizeof(ws_mutex_t));
	if (mut == NULL) return NULL;
	InitializeCriticalSection(mut);
	return mut;
}

void ws_destroyMutex(ws_mutex_t* mut) {
	ws_lockMutex(mut);
	DeleteCriticalSection(mut);
	free(mut);
}

ws_thread_id_t ws_getThreadID() { return GetCurrentThreadId(); }
void ws_internal_printThreadID(FILE* stream) { fprintf(stream, "%lu", ws_getThreadID()); }

#else

void ws_lockMutex(ws_mutex_t* mut) { pthread_mutex_lock(mut); }
void ws_unlockMutex(ws_mutex_t* mut) { pthread_mutex_unlock(mut); }
ws_mutex_t* ws_createMutex() {
	ws_mutex_t* mut = (ws_mutex_t*) malloc(sizeof(ws_mutex_t));
	if (mut == NULL) return NULL;
	pthread_mutex_init(mut, NULL);
	return mut;
}
void ws_destroyMutex(ws_mutex_t* mut) {
	ws_lockMutex(mut);
	pthread_mutex_destroy(mut);
	free(mut);
}
ws_thread_id_t ws_getThreadID() { return pthread_self(); }
void ws_internal_printThreadID(FILE* stream) { fprintf(stream, "%zu", ws_getThreadID()); }
#endif // _WIN32
#endif // CUSTOM_THREADS

typedef struct {
	bool b;
	ws_mutex_t* mut;
} ws_atomic_bool_t;

bool ws_getAtomicBool(ws_atomic_bool_t* ab) {
	bool ret;
	ws_lockMutex(ab->mut);
	ret = ab->b;
	ws_unlockMutex(ab->mut);
	return ret;
}

void ws_setAtomicBool(ws_atomic_bool_t* ab, bool b) {
	ws_lockMutex(ab->mut);
	ab->b = b;
	ws_unlockMutex(ab->mut);
}

ws_atomic_bool_t* ws_createAtomicBool(bool b) {
	ws_atomic_bool_t* ab = (ws_atomic_bool_t*) malloc(sizeof(ws_atomic_bool_t));
	if (ab == NULL) return NULL;
	ab->b = b;
	ab->mut = ws_createMutex();
	return ab;
}

void ws_destroyAtomicBool(ws_atomic_bool_t* ab) {
	ws_lockMutex(ab->mut);
	ws_destroyMutex(ab->mut);
	free(ab);
}

#ifdef WALLSHELL_LOGGING
typedef struct {
	char* name;
	ws_thread_id_t id;
} ws_thread_map_t;

ws_thread_map_t* thread_map = NULL;
size_t thread_map_size = 0;
size_t thread_map_current = 0;

ws_mutex_t* thread_map_mut = NULL;

void ws_addThreadName(char* name) {
	if (!thread_map_mut) {
		thread_map_mut = ws_createMutex();
		if (!thread_map_mut) return;
	}
	ws_lockMutex(thread_map_mut);
	if (!thread_map) {
		thread_map = (ws_thread_map_t*) calloc(1, sizeof(ws_thread_map_t));
		if (!thread_map) return;
		thread_map_size++;
	}
	if (thread_map_size - 1 < thread_map_current) {
		ws_thread_map_t* temp = realloc(thread_map, (thread_map_size + 1) * sizeof(ws_thread_map_t));
		if (!temp) return;
		thread_map = temp;
		thread_map_size++;
	}
	char* thread_name = calloc(strlen(name), sizeof(char));
	if (!thread_name) return;
#ifndef _WIN32
	strcpy(thread_name, name);
#else
	strcpy_s(thread_name, strlen(name), name);
#endif
	thread_map[thread_map_current].name = thread_name;
	thread_map[thread_map_current].id = ws_getThreadID();
	
	thread_map_current++;
	ws_unlockMutex(thread_map_mut);
}

void ws_removeThreadName(const char* name) {
	if (!thread_map_mut) {
		thread_map_mut = ws_createMutex();
		if (!thread_map_mut) return;
	}
	ws_lockMutex(thread_map_mut);
	
	if (!thread_map) return;
	if (thread_map_current == 0) return;
	if (thread_map_size == 0) return; // This should be impossible.
	
	for (int i = 0; i < thread_map_current; i++) {
		if (strcmp(thread_map[thread_map_current].name, name) == 0) {
			free(thread_map[thread_map_current].name);
			for (int j = i; j < thread_map_current - 1; j++) {
				thread_map[j].id = thread_map[j + 1].id;
				thread_map[j].name = thread_map[j + 1].name;
			}
			ws_unlockMutex(thread_map_mut);
			return;
		}
	}
	
	wallshell_unlockMutex(thread_map_mut);
}

void wallshell_printThreadID() {
	if (!thread_map_mut) {
		thread_map_mut = wallshell_createMutex();
		if (!thread_map_mut) return;
	}
	wallshell_lockMutex(thread_map_mut);
	wallshell_thread_id_t cur = wallshell_getThreadID();
	for (int i = 0; i < thread_map_current; i++) {
		if (thread_map[i].id == cur) {
			fprintf(wallshell_out_stream, "%s", thread_map[i].name);
			goto exit;
		}
	}
	wallshell_internal_printThreadID(wallshell_out_stream);
exit:
	wallshell_unlockMutex(thread_map_mut);
}

void wallshell_internal_cleanThreads() {
	for (int i = 0; i < thread_map_current; i++) {
		free(thread_map[i].name);
	}
	free(thread_map);
	thread_map = NULL;
}

void wallshell_log(const char* f, ...) {
	wallshell_lockMutex(internal_mutex);
	
	fprintf(wallshell_out_stream, "[");
	wallshell_printThreadID();
	fprintf(wallshell_out_stream, "] ");
	va_list args;
			va_start(args, f);
	vfprintf(wallshell_out_stream, f, args);
			va_end(args);
	fprintf(wallshell_out_stream, "\n");
	
	wallshell_unlockMutex(internal_mutex);
}
#endif // WALLSHELL_LOGGING

void otherThread() {
	wallshell_addThreadName("Thread 2");
	for (int i = 0; i < 50; i++) {
		wallshell_log("%d", i);
	}
}

int main() {
	wallshell_out_stream = stdout;
	internal_mutex = wallshell_createMutex();

#ifdef _WIN32
	HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) otherThread, NULL, 0, NULL);
#else
	pthread_t thread;
	pthread_create(&thread, NULL, otherThread, NULL);
#endif
	
	wallshell_addThreadName("Main");
	for (int i = 0; i < 50; i++) {
		wallshell_log("%d", i);
	}

#ifdef _WIN32
	WaitForSingleObject(thread, INFINITE);
#else
	pthread_join(thread, NULL);
#endif
}