#ifndef BASE_ATOMIC_H
#define BASE_ATOMIC_H

#if OS_WINDOWS
# define atomic_eval(ptr) InterlockedAdd64((volatile long long *)(ptr), 0)
# define atomic_inc(ptr) InterlockedIncrement64((volatile long long *)(ptr))
#elif OS_LINUX
# define atomic_eval(ptr)
# define atomic_inc(ptr)
#elif OS_ANDROID
# define atomic_eval(ptr)
# define atomic_inc(ptr)
#else
# error no atomic operations defined for this OS
#endif

#if COMPILER_CLANG || COMPILER_GCC
# define memory_fence() asm("" ::: "memory")
#elif COMPILER_CL
# define memory_fence() MemoryBarrier()
#else
# error "There isn't a memory fence defined for this compiler."
#endif

#endif // BASE_ATOMIC_H
