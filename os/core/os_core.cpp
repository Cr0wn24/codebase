//////////////////////////////
// NOTE(hampus): Handle functions

static OS_Handle
os_handle_zero()
{
  OS_Handle result = {};
  return result;
}

static B32
os_handle_match(OS_Handle a, OS_Handle b)
{
  B32 result = a.u64[0] == b.u64[0];
  return result;
}
