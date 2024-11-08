#ifndef BASE_CORE_H
#define BASE_CORE_H

#define _CRT_SECURE_NO_WARNINGS
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

//////////////////////////////
// NOTE(hampus): Context cracking

#if defined(BUILDING_ANDROID)
# undef _WIN32

# ifndef __ANDROID__
#  define __ANDROID__
# endif

#endif

#if defined(__clang__)
# define COMPILER_CLANG 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# elif defined(__gnu_linux__)
#  define OS_LINUX 1
# elif defined(__APPLE__) && defined(__MACH__)
#  define OS_MAC 1
# elif defined(__ANDROID__)
#  define OS_ANDROID 1
# else
#  error missing OS detection
# endif

# if defined(__amd64__)
#  define ARCH_X64 1
# elif defined(__i386__)
#  define ARCH_X86 1
# elif defined(__arm__)
#  define ARCH_ARM 1
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# else
#  error missing ARCH detection
# endif

#elif defined(_MSC_VER)
# define COMPILER_CL 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# else
#  error missing OS detection
# endif

# if defined(_M_AMD64)
#  define ARCH_X64 1
# elif defined(_M_IX86)
#  define ARCH_X86 1
# elif defined(_M_ARM)
#  define ARCH_ARM
# else
#  error missing ARCH detection
# endif

#elif defined(__GNUC__)
# define COMPILER_GCC 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# elif defined(__gnu_linux__)
#  define OS_LINUX 1
# elif defined(__APPLE__) && defined(__MACH__)
#  define OS_MAC 1
# else
#  error missing OS detection
# endif

# if defined(__amd64__)
#  define ARCH_X64 1
# elif defined(__i386__)
#  define ARCH_X86 1
# elif defined(__arm__)
#  define ARCH_ARM
# elif defined(__aarch64__)
#  define ARCH_ARM64
# else
#  error missing ARCH detection
# endif

#endif

#if !defined(COMPILER_CL)
# define COMPILER_CL 0
#endif

#if !defined(COMPILER_CLANG)
# define COMPILER_CLANG 0
#endif

#if !defined(COMPILER_GCC)
# define COMPILER_GCC 0
#endif

#if !defined(OS_WINDOWS)
# define OS_WINDOWS 0
#endif

#if !defined(OS_LINUX)
# define OS_LINUX 0
#endif

#if !defined(OS_MAC)
# define OS_MAC 0
#endif

#if !defined(OS_ANDROID)
# define OS_ANDROID 0
#endif

#if !defined(ARCH_X64)
# define ARCH_X64 0
#endif

#if !defined(ARCH_X86)
# define ARCH_X86 0
#endif

#if !defined(ARCH_ARM)
# define ARCH_ARM 0
#endif

#if !defined(ARCH_ARM64)
# define ARCH_ARM64 0
#endif

static_assert(ARCH_ARM64 || ARCH_X64, "This architecture is not supported");

#define PRIU8 PRIu8
#define PRIU16 PRIu16
#define PRIU32 PRIu32
#define PRIU64 PRIu64
#define PRIS8 PRId8
#define PRIS16 PRId16
#define PRIS32 PRId32
#define PRIS64 PRId64

//////////////////////////////
// NOTE(hampus): Read only macro

#if COMPILER_CL || (COMPILER_CLANG && OS_WINDOWS)
# pragma section(".rdata$", read)
# define read_only __declspec(allocate(".rdata$"))
#elif COMPILER_CLANG
# define read_only __attribute__((section(".rodata")))
#else
# define read_only
#endif

//////////////////////////////
// NOTE(hampus): Linked lists macros

#define CheckNil(nil, p) ((p) == 0 || (p) == nil)
#define SetNil(nil, p) ((p) = nil)

#define DLLInsertNPZ(nil, f, l, p, n, next, prev) (CheckNil(nil, f)                                                    \
                                                   ? ((f) = (l) = (n), SetNil(nil, (n)->next), SetNil(nil, (n)->prev)) \
                                                   : CheckNil(nil, p) ? ((n)->next = (f), (f)->prev = (n), (f) = (n),  \
                                                                         SetNil(nil, (n)->prev))                       \
                                                     : ((p) == (l))                                                    \
                                                       ? ((l)->next = (n), (n)->prev = (l), (l) = (n),                 \
                                                          SetNil(nil, (n)->next))                                      \
                                                       : (((!CheckNil(nil, p) && CheckNil(nil, (p)->next))             \
                                                           ? (0)                                                       \
                                                           : ((p)->next->prev = (n))),                                 \
                                                          ((n)->next = (p)->next), ((p)->next = (n)), ((n)->prev = (p))))
#define DLLPushBackNPZ(nil, f, l, n, next, prev) \
 DLLInsertNPZ(nil, f, l, l, n, next, prev)
#define DLLPushFrontNPZ(nil, f, l, n, next, prev) \
 DLLInsertNPZ(nil, l, f, f, n, prev, next)
#define DLLRemoveNPZ(nil, f, l, n, next, prev)                                \
 (((n) == (f) ? (f) = (n)->next : (0)), ((n) == (l) ? (l) = (l)->prev : (0)), \
  (CheckNil(nil, (n)->prev) ? (0) : ((n)->prev->next = (n)->next)),           \
  (CheckNil(nil, (n)->next) ? (0) : ((n)->next->prev = (n)->prev)))

#define SLLQueuePushNZ(nil, f, l, n, next)                     \
 (CheckNil(nil, f) ? ((f) = (l) = (n), SetNil(nil, (n)->next)) \
                   : ((l)->next = (n), (l) = (n), SetNil(nil, (n)->next)))
#define SLLQueuePushFrontNZ(nil, f, l, n, next)                \
 (CheckNil(nil, f) ? ((f) = (l) = (n), SetNil(nil, (n)->next)) \
                   : ((n)->next = (f), (f) = (n)))
#define SLLQueuePopNZ(nil, f, l, next) \
 ((f) == (l) ? (SetNil(nil, f), SetNil(nil, l)) : ((f) = (f)->next))

#define SLLStackPushN(f, n, next) ((n)->next = (f), (f) = (n))
#define SLLStackPopN(f, next) ((f) = (f)->next)

#define DLLInsertNP(f, l, p, n, next, prev) \
 DLLInsertNPZ(0, f, l, p, n, next, prev)
#define DLLPushBackNP(f, l, n, next, prev) \
 DLLPushBackNPZ(0, f, l, n, next, prev)
#define DLLPushFrontNP(f, l, n, next, prev) \
 DLLPushFrontNPZ(0, f, l, n, next, pre, v)
#define DLLRemoveNP(f, l, n, next, prev) \
 DLLRemoveNPZ(0, f, l, n, next, prev)
#define DLLInsert(f, l, p, n) DLLInsertNPZ(0, f, l, p, n, next, prev)
#define DLLPushBack(f, l, n) DLLPushBackNPZ(0, f, l, n, next, prev)
#define DLLPushFront(f, l, n) DLLPushFrontNPZ(0, f, l, n, next, prev)
#define DLLRemove(f, l, n) DLLRemoveNPZ(0, f, l, n, next, prev)

#define SLLQueuePushN(f, l, n, next) SLLQueuePushNZ(0, f, l, n, next)
#define SLLQueuePushFrontN(f, l, n, next) \
 SLLQueuePushFrontNZ(0, f, l, n, next)
#define SLLQueuePopN(f, l, next) SLLQueuePopNZ(0, f, l, next)
#define SLLQueuePush(f, l, n) SLLQueuePushNZ(0, f, l, n, next)
#define SLLQueuePushFront(f, l, n) SLLQueuePushFrontNZ(0, f, l, n, next)
#define SLLQueuePop(f, l) SLLQueuePopNZ(0, f, l, next)

#define SLLStackPush(f, n) SLLStackPushN(f, n, next)
#define SLLStackPop(f) SLLStackPopN(f, next)

//////////////////////////////
// NOTE(hampus): Base types

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef int8_t S8;
typedef int16_t S16;
typedef int32_t S32;
typedef int64_t S64;

typedef U8 B8;
typedef S16 B16;
typedef S32 B32;
typedef S64 B64;

typedef float F32;
typedef double F64;

//////////////////////////////
// NOTE(hampus): Basic helper macros

#if COMPILER_CLANG
# define debug_break() __builtin_debugtrap()
#elif COMPILER_CL
# define debug_break() DebugBreak()
#elif COMPILER_GCC
# define debug_break() __builtin_trap()
#else
# define debug_break() (*(volatile int *)0 = 0)
#endif

#if COMPILER_CL
# define per_thread __declspec(thread)
#elif COMPILER_GCC
# define per_thread __thread
#elif COMPILER_CLANG
# define per_thread __thread
#else
# error no per_thread exists for this compiler
#endif

#define AssertAlways(expr) \
 if(!(expr)) [[unlikely]]  \
  (*(volatile int *)0 = 0);

#define Assert(expr) \
 if(!(expr))         \
  (*(volatile int *)0 = 0);

#define invalid_case       \
 default:                  \
 {                         \
  Assert(!"Invalid case"); \
 }                         \
 break;
#define InvalidCodePath Assert(!"Invalid code path")
#define NotImplemented Assert(!"Not implemented")

#define Stringify_(s) #s
#define Stringify(s) Stringify_(s)

#define Glue_(a, b) a##b
#define Glue(a, b) Glue_(a, b)

#define IntFromPtr(p) (unsigned long long)((char *)(p) - (char *)0)
#define PtrFromInt(n) (void *)((char *)0 + (n))

#define DeferLoopChecked(begin, end)                               \
 for(S32 Glue(_i_, __LINE__) = 2 * !(begin);                       \
     Glue(_i_, __LINE__) == 2 ? ((end), 0) : !Glue(_i_, __LINE__); \
     ++Glue(_i_, __LINE__), (end))
#define DeferLoop(begin, end)                                      \
 for(S32 Glue(_i_, __LINE__) = ((begin), 0); !Glue(_i_, __LINE__); \
     ++Glue(_i_, __LINE__), (end))

#define ForEachEnumVal(e, var) for(e var = static_cast<e>(0); var < e##_COUNT; var = static_cast<e>(static_cast<int>(var) + 1))

#define Member(t, m) (((t *)0)->m)
#define MemberOffset(t, m) offsetof(t, m)

#define Kilobytes(n) ((n) * 1024LL)
#define Megabytes(n) (1024LL * Kilobytes(n))
#define Gigabytes(n) (1024LL * Megabytes(n))
#define Terabytes(n) (1024LL * Gigabytes(n))

#define Thousand(n) ((n) * 1000)
#define Million(n) ((Thousand(n)) * 1000)
#define billion(n) ((Million(n)) * 1000)

#define AxisFlip(axis) ((axis) == Axis2_X ? Axis2_Y : Axis2_X)
enum Axis2
{
 Axis2_X,
 Axis2_Y,

 Axis2_COUNT,
};

enum Axis3
{
 Axis3_X,
 Axis3_Y,
 Axis3_Z,

 Axis3_COUNT,
};

enum Axis4
{
 Axis4_X,
 Axis4_Y,
 Axis4_Z,
 Axis4_W,

 Axis4_COUNT,
};

#define SideFlip(side) (!(side))
enum Side
{
 Side_Min,
 Side_Max,

 Side_COUNT,
};

enum Corner
{
 Corner_TopLeft,
 Corner_TopRight,
 Corner_BottomLeft,
 Corner_BottomRight,

 Corner_COUNT,
};

enum OperatingSystem
{
 OperatingSystem_Null,
 OperatingSystem_Windows,
 OperatingSystem_Linux,
 OperatingSystem_Mac,

 OperatingSystem_COUNT,
};

enum Architecture
{
 Architecture_Null,
 Architecture_X64,
 Architecture_X86,
 Architecture_ARM,
 Architecture_ARM64,

 Architecture_COUNT,
};

enum Month
{
 Month_Jan,
 Month_Feb,
 Month_Mar,
 Month_Apr,
 Month_May,
 Month_Jun,
 Month_Jul,
 Month_Aug,
 Month_Sep,
 Month_Oct,
 Month_Nov,
 Month_Dec,

 Month_COUNT
};

enum DayOfWeek
{
 DayOfWeek_Monday,
 DayOfWeek_Tuesday,
 DayOfWeek_Wednesday,
 DayOfWeek_Thursday,
 DayOfWeek_Friday,
 DayOfWeek_Saturday,
 DayOfWeek_Sunday,

 DayOfWeek_COUNT,
};

struct DenseTime
{
 U64 time;
};

struct DateTime
{
 U16 millisecond;
 U8 second;
 U8 minute;
 U8 hour;
 U8 day;
 U8 month;
 S16 year;
};

struct Date
{
 U8 day;
 U8 month;
 S16 year;
};

struct String8;

[[nodiscard]] static OperatingSystem os_from_context();
[[nodiscard]] static Architecture arch_from_context();
[[nodiscard]] static DateTime build_date_from_context();

[[nodiscard]] static String8 string_from_arch(Architecture arc);
[[nodiscard]] static String8 string_from_os(OperatingSystem os);
[[nodiscard]] static String8 string_from_day_of_week(DayOfWeek day);
[[nodiscard]] static String8 string_from_month(Month month);

[[nodiscard]] static DenseTime dense_time_from_date_time(DateTime date_time);
[[nodiscard]] static DateTime date_time_from_dense_time(DenseTime dense_time);
[[nodiscard]] static B32 date_match(Date a, Date b);

struct MemorySize
{
 F64 amount;
 U8 *unit;
 U64 unit_length;
};

[[nodiscard]] static MemorySize memory_size_from_bytes(U64 bytes);

#endif // BASE_CORE_H
