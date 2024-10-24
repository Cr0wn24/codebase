function void
fp_init(void)
{
  Arena *arena = arena_alloc();
  fp_state = push_array<FP_State>(arena, 1);
  fp_state->arena = arena;
  fp_state->read_file = os_read_file;
}

function void
fp_destroy(void)
{
  arena_free(fp_state->arena);
  fp_state = 0;
}

function B32
fp_handle_match(FP_Handle a, FP_Handle b)
{
  B32 result = memory_match(&a, &b, sizeof(FP_Handle));
  return result;
}

function FP_Handle
fp_handle_zero(void)
{
  FP_Handle result = {};
  return result;
}

function void
fp_set_file_read_proc(FP_FileReadProc *proc)
{
  fp_state->read_file = proc;
}