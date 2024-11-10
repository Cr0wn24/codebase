//////////////////////////////
// NOTE(hampus): Globals

per_thread ThreadCtx *thread_ctx;

//////////////////////////////
// NOTE(hampus): Thread ctx functions

static ThreadCtx *
thread_ctx_init(String8 name)
{
  thread_ctx = thread_ctx_alloc();
  set_thread_ctx(thread_ctx);
  set_thread_ctx_name(name);
  return (thread_ctx);
}

static ThreadCtx *
thread_ctx_alloc()
{
  Arena *arena = arena_alloc();
  ThreadCtx *result = push_array<ThreadCtx>(arena, 1);
  result->permanent_arena = arena;
  for(U64 i = 0; i < array_count(result->scratch_arenas); ++i)
  {
    result->scratch_arenas[i] = arena_alloc();
  }
  return result;
}

static void
thread_ctx_release(ThreadCtx *tctx)
{
  for(U64 i = 0; i < array_count(tctx->scratch_arenas); ++i)
  {
    arena_free(tctx->scratch_arenas[i]);
  }
}

static void
set_thread_ctx(ThreadCtx *tctx)
{
  thread_ctx = tctx;
}

static ThreadCtx *
get_thread_ctx()
{
  ThreadCtx *result = thread_ctx;
  return result;
}

static void
set_thread_ctx_name(String8 string)
{
  ThreadCtx *ctx = get_thread_ctx();
  Assert(string.size + 1 <= array_count(ctx->name));
  MemoryCopy(ctx->name.val, string.data, string.size);
  ctx->name[string.size] = 0;
  if(string.size != 0)
  {
    os_thread_set_name(string);
  }
}

static String8
get_thread_ctx_name()
{
  ThreadCtx *ctx = get_thread_ctx();
  String8 result = str8_cstr((char *)ctx->name.val);
  return result;
}

//////////////////////////////
// NOTE(hampus): Scratch functions

static Arena *
get_scratch_arena(Arena **conflicts, U32 count)
{
  Arena *selected = 0;
  ThreadCtx *tctx = get_thread_ctx();
  for(U64 i = 0; i < array_count(tctx->scratch_arenas); ++i)
  {
    Arena *arena = tctx->scratch_arenas[i];

    B32 is_non_conflicting = true;
    for(U64 j = 0; j < count; ++j)
    {
      if(arena == conflicts[j])
      {
        is_non_conflicting = false;
        break;
      }
    }
    if(is_non_conflicting)
    {
      selected = arena;
      break;
    }
  }
  return selected;
}