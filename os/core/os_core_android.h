#ifndef OS_CORE_ANDROID_H
#define OS_CORE_ANDROID_H

#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <execinfo.h>
#include <unwind.h>
#include <dlfcn.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <semaphore.h>

#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android_native_app_glue.h>
#include <android/sensor.h>
#include <android/permission_manager.h>
#include <jni.h>

#define SETUP_FOR_JAVA_CALL_THREAD          \
  JNIEnv *env = android_app->activity->env; \
  JavaVM *vm = android_app->activity->vm;   \
  vm->AttachCurrentThread(&env, 0);

#define JAVA_CALL_DETACH_THREAD vm->DetachCurrentThread();

struct OS_Android_State
{
  Arena *arena;
  OS_CrashHandlerProc *crash_handler_proc;
};

struct OS_Android_ThreadArgs
{
  ThreadProc *proc;
  void *data;
};

struct OS_Android_Mutex
{
  pthread_mutex_t mutex;
};

struct OS_Android_Semaphore
{
  sem_t semaphore;
};

#define MAX_ADDRESS_COUNT 30

struct OS_Android_BacktraceState
{
  const ucontext_t *signal_ucontext;
  U64 address_count;
  StaticArray<uintptr_t, MAX_ADDRESS_COUNT> addresses;
  OS_Backtrace *backtrace;
};

global OS_Android_State *os_android_state;
global struct android_app *android_app;

void os_android_sig_action_handler(int sig, siginfo_t *info, void *ucontext);

#endif