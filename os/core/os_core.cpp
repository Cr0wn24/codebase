//////////////////////////////
// NOTE(hampus): Handle functions

function OS_Handle
os_handle_zero(void)
{
  OS_Handle result = {};
  return result;
}

function B32
os_handle_match(OS_Handle a, OS_Handle b)
{
  B32 result = a.u64[0] == b.u64[0];
  return result;
}
