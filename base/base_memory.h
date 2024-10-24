#ifndef BASE_MEMORY_H
#define BASE_MEMORY_H

#if COMPILER_CL
# pragma warning(push, 0)
#elif COMPILER_CLANG
#endif

#include <sanitizer/asan_interface.h>

#if COMPILER_CL
# pragma warning(pop)
#elif COMPILER_CLANG
#endif

//////////////////////////////
// NOTE(hampus): Defines

#define ARENA_DEFAULT_RESERVE_SIZE gigabytes(2)
#define ARENA_COMMIT_BLOCK_SIZE kilobytes(64)

//////////////////////////////
// NOTE(hampus): Arena types

struct Arena
{
 U8 *memory;
 U64 cap;
 U64 pos;
 U64 commit_pos;
};

struct TempArena
{
 Arena *arena;
 U64 pos;
 TempArena(Arena *_arena);
 ~TempArena();
};

//////////////////////////////
// NOTE(hampus): Base functions

function Arena *arena_alloc(void);
function void arena_free(Arena *arena);

function void *arena_push(Arena *arena, U64 size);
function void *arena_push_no_zero(Arena *arena, U64 size);
function void arena_pop_to(Arena *arena, U64 pos);
function void arena_pop_amount(Arena *arena, U64 amount);

function void arena_align(Arena *arena, U64 power);
function void arena_align_no_zero(Arena *arena, U64 power);

//////////////////////////////
// NOTE(hampus): Ring

#define ring_write_struct(b, sz, o, s) ring_write(b, sz, o, str8_struct(s))
#define ring_read_struct(b, sz, o, s) ring_read(b, sz, o, str8_struct(s))

function U64 ring_write(U8 *base, U64 size, U64 offset, String8 string);
function U64 ring_read(U8 *base, U64 size, U64 offset, String8 string);

//////////////////////////////
// NOTE(hampus): Macro wrappers

template <typename T>
function T *
push_array(Arena *arena, U64 count)
{
 T *result = (T *)arena_push(arena, sizeof(T) * count);
 return result;
}

template <typename T>
function T *
push_array_no_zero(Arena *arena, U64 count)
{
 T *result = (T *)arena_push_no_zero(arena, sizeof(T) * count);
 return result;
}

#define memory_zero(dst, size) memset((dst), 0, (size))
#define memory_zero_struct(dst) memory_zero((dst), sizeof(*(dst)))
#define memory_zero_array(dst, count) memory_zero((dst), sizeof(*dst) * count)

#define memory_match(a, b, size) (memcmp((a), (b), (size)) == 0)

#define memory_move(dst, source, size) memmove((dst), (source), size)

#define memory_copy(dst, source, size) memcpy((U8 *)(dst), (U8 *)(source), size)
#define memory_copy_struct(dst, source) memory_copy((dst), (source), min(sizeof(*(dst)), sizeof(*(source))))
#define memory_copy_array(dst, source) memory_copy((dst), (source), min(sizeof(dst), sizeof(source)))

#define arena_clear(arena) arena_pop_to(arena, 0)

#endif // BASE_MEMORY_H
