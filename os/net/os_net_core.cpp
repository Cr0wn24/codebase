static OS_NetAddress
os_net_address_from_str8(String8 string)
{
  OS_NetAddress address = os_net_address_zero();
  TempArena scratch = get_scratch(0, 0);
  U64 colon_idx = 0;
  str8_first_index_of(string, ':', &colon_idx);
  String8 ip_string = str8_prefix(string, colon_idx);
  String8List ip_segments = str8_split_by_codepoints(scratch.arena, ip_string, str8_lit(" "));
  U8 *dst = address.ip.u8;
  for(String8Node *n = ip_segments.first; n != 0; n = n->next)
  {
    u8_from_str8(n->v, dst);
    dst++;
  }
  String8 port_string = str8_skip(string, colon_idx + 1);
  u16_from_str8(port_string, &address.port);
  return (address);
}

static String8
os_str8_from_net_address(Arena *arena, OS_NetAddress address)
{
  String8 string = str8_push(arena, (char *)"%d.%d.%d.%d:%d",
                             address.ip.u8[0], address.ip.u8[1], address.ip.u8[2], address.ip.u8[3],
                             address.port);
  return (string);
}

static B32
os_net_address_match(OS_NetAddress a, OS_NetAddress b)
{
  B32 result = a.port == b.port && a.ip.u32[0] == b.ip.u32[0];
  return result;
}

static OS_NetAddress
os_net_address_zero()
{
  OS_NetAddress result = {};
  return result;
}