#ifndef BASE_THREAD_CTX_H
#define BASE_THREAD_CTX_H

//////////////////////////////
// NOTE(hampus): Defines

#define THREAD_SCRATCH_ARENA_POOL_SIZE 4
#define THREAD_CONTEXT_NAME_SIZE 16

//////////////////////////////
// NOTE(hampus): Types

struct ThreadCtx
{
  Arena *permanent_arena;
  StaticArray<Arena *, THREAD_SCRATCH_ARENA_POOL_SIZE> scratch_arenas;
  StaticArray<U8, THREAD_CONTEXT_NAME_SIZE> name;
};

//////////////////////////////
// NOTE(hampus): Functions

static ThreadCtx *thread_ctx_init(String8 name);

[[nodiscard]] static ThreadCtx *thread_ctx_alloc();
static void thread_ctx_release(ThreadCtx *tctx);
static void set_thread_ctx(ThreadCtx *tctx);
[[nodiscard]] static ThreadCtx *get_thread_ctx();

static void set_thread_ctx_name(String8 string);
[[nodiscard]] static String8 get_thread_ctx_name();

//////////////////////////////
// NOTE(hampus): Scratch functions

#define GetScratch(conflicts, count) TempArena(get_scratch_arena(conflicts, count))
#define release_scratch(scratch) end_temp_arena(scratch)
[[nodiscard]] static Arena *get_scratch_arena(Arena **conflicts, U32 count);

#endif // BASE_THREAD_CTX_H
