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
  Array<Arena *, THREAD_SCRATCH_ARENA_POOL_SIZE> scratch_arenas;
  Array<U8, THREAD_CONTEXT_NAME_SIZE> name;
};

//////////////////////////////
// NOTE(hampus): Functions

function ThreadCtx *thread_ctx_init(String8 name);

function ThreadCtx *thread_ctx_alloc(void);
function void thread_ctx_release(ThreadCtx *tctx);
function void set_thread_ctx(ThreadCtx *tctx);
function ThreadCtx *get_thread_ctx(void);

function void set_thread_ctx_name(String8 string);
function String8 get_thread_ctx_name(void);

//////////////////////////////
// NOTE(hampus): Scratch functions

#define get_scratch(conflicts, count) TempArena(get_scratch_arena(conflicts, count))
#define release_scratch(scratch) end_temp_arena(scratch)
function Arena *get_scratch_arena(Arena **conflicts, U32 count);

#endif // BASE_THREAD_CTX_H
