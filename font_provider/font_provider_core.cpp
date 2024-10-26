static void
fp_destroy()
{
  arena_free(fp_state->arena);
  fp_state = 0;
}

static B32
fp_handle_match(FP_Handle a, FP_Handle b)
{
  B32 result = memory_match(&a, &b, sizeof(FP_Handle));
  return result;
}

static FP_Handle
fp_handle_zero()
{
  FP_Handle result = {};
  return result;
}