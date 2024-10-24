#ifndef OS_NET_CORE_H
#define OS_NET_CORE_H

enum OS_NetProtocol
{
  OS_NetProtocol_TCP,
  OS_NetProtocol_UDP,
};

// TODO(hampus): INet6, UNIX, ...
enum OS_NetAddressFamily
{
  OS_NetAddressFamily_INet,
};

struct OS_NetAddress
{
  U16 port;
  union
  {
    U32 u32[1];
    U8 u8[4];
  } ip;
  OS_NetAddressFamily address_family;
};

struct OS_NetReceiveResult
{
  String8 data;
  OS_NetAddress address;
};

struct OS_NetAcceptResult
{
  OS_Handle socket;
  OS_NetAddress address;
  B32 succeeded;
};

[[nodiscard]] function OS_NetAddress os_net_address_from_str8(String8 string);
[[nodiscard]] function String8 os_str8_from_net_address(Arena *arena, OS_NetAddress address);
[[nodiscard]] function B32 os_net_address_match(OS_NetAddress a, OS_NetAddress b);
[[nodiscard]] function OS_NetAddress os_net_address_zero(void);

function void os_net_init(void);
[[nodiscard]] function OS_Handle os_socket_alloc(OS_NetProtocol protocol, OS_NetAddressFamily address_family);
[[nodiscard]] function B32 os_socket_free(OS_Handle socket);
[[nodiscard]] function B32 os_socket_bind(OS_Handle socket, OS_NetAddress address);
[[nodiscard]] function B32 os_socket_set_blocking_mode(OS_Handle socket, B32 should_block);
[[nodiscard]] function OS_NetAddress os_address_from_socket(OS_Handle socket);

[[nodiscard]] function B32 os_socket_connect(OS_Handle socket, OS_NetAddress address);
[[nodiscard]] function OS_NetAcceptResult os_socket_accept(OS_Handle socket);
[[nodiscard]] function B32 os_socket_send(OS_Handle socket, String8 data);
[[nodiscard]] function OS_NetReceiveResult os_socket_receive(Arena *arena, OS_Handle connected_socket, U64 cap);

[[nodiscard]] function B32 os_socket_send_to(OS_Handle socket, OS_NetAddress address, String8 data);
[[nodiscard]] function OS_NetReceiveResult os_socket_receive_from(Arena *arena, OS_Handle listen_socket, U64 cap);

#endif // OS_NET_CORE_H
