#pragma comment(lib, "ws2_32.lib")

static String8
os_net_get_error_string()
{
  S32 error_id = WSAGetLastError();
  U8 buffer[1024] = {};
  U64 size = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                            0, safe_u32_from_s32(error_id), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buffer, 1024, 0);
  String8 result = str8(buffer, size);
  return result;
}

static struct sockaddr_in
os_win32_sockaddr_in_from_net_address(OS_NetAddress address)
{
  struct sockaddr_in result = {};
  result.sin_addr.S_un.S_un_b.s_b1 = address.ip.u8[0];
  result.sin_addr.S_un.S_un_b.s_b2 = address.ip.u8[1];
  result.sin_addr.S_un.S_un_b.s_b3 = address.ip.u8[2];
  result.sin_addr.S_un.S_un_b.s_b4 = address.ip.u8[3];
  result.sin_port = htons(address.port);
  switch(address.address_family)
  {
    case OS_NetAddressFamily_INet:
    {
      result.sin_family = AF_INET;
    }
    break;
  }
  return result;
}

static OS_NetAddress
os_win32_net_address_from_sockaddr_in(struct sockaddr_in sockaddr)
{
  OS_NetAddress result = {};
  result.ip.u8[0] = sockaddr.sin_addr.S_un.S_un_b.s_b1;
  result.ip.u8[1] = sockaddr.sin_addr.S_un.S_un_b.s_b2;
  result.ip.u8[2] = sockaddr.sin_addr.S_un.S_un_b.s_b3;
  result.ip.u8[3] = sockaddr.sin_addr.S_un.S_un_b.s_b4;
  result.port = ntohs(sockaddr.sin_port);
  switch(sockaddr.sin_family)
  {
    case AF_INET:
    {
      result.address_family = OS_NetAddressFamily_INet;
    }
    break;
  }
  return result;
}

static void
os_net_init()
{
  WSADATA wsa_data;
  WSAStartup(MAKEWORD(2, 2), &wsa_data);
}

static OS_Handle
os_socket_alloc(OS_NetProtocol protocol, OS_NetAddressFamily address_family)
{
  OS_Handle result = os_handle_zero();
  S32 ipproto = 0;
  S32 stream = 0;
  switch(protocol)
  {
    case OS_NetProtocol_TCP:
    {
      ipproto = IPPROTO_TCP;
      stream = SOCK_STREAM;
    }
    break;
    case OS_NetProtocol_UDP:
    {
      ipproto = IPPROTO_UDP;
      stream = SOCK_DGRAM;
    }
    break;
      InvalidCase;
  }
  S32 af = 0;
  switch(address_family)
  {
    case OS_NetAddressFamily_INet:
    {
      af = AF_INET;
    }
    break;
      InvalidCase;
  }
  SOCKET sock = socket(af, stream, ipproto);
  if(sock != INVALID_SOCKET)
  {
    result.u64[0] = (U64)sock;
  }
  return result;
}

static B32
os_socket_free(OS_Handle socket)
{
  SOCKET sock = (SOCKET)socket.u64[0];
  S32 closesocket_result = closesocket(sock);
  B32 result = closesocket_result != SOCKET_ERROR;
  return result;
}

static B32
os_socket_bind(OS_Handle socket, OS_NetAddress address)
{
  struct sockaddr_in sockaddrin = os_win32_sockaddr_in_from_net_address(address);
  SOCKET sock = (SOCKET)socket.u64[0];
  S32 bind_result = bind(sock, (struct sockaddr *)&sockaddrin, sizeof(sockaddrin));
  B32 result = bind_result != SOCKET_ERROR;
  return result;
}

static B32
os_socket_set_blocking_mode(OS_Handle socket, B32 should_block)
{
  u_long mode = (u_long)(should_block ? 0 : 1);
  SOCKET sock = (SOCKET)socket.u64[0];
  S32 ioctlsocket_result = ioctlsocket(sock, FIONBIO, &mode);
  B32 result = ioctlsocket_result != SOCKET_ERROR;
  return result;
}

static OS_NetAddress
os_address_from_socket(OS_Handle socket)
{
  OS_NetAddress result = {};
  SOCKET s = (SOCKET)socket.u64[0];
  struct sockaddr_in addr = {};
  S32 addrlen = sizeof(addr);
  S32 getsockname_result = getsockname(s, (struct sockaddr *)&addr, &addrlen);
  if(getsockname_result != SOCKET_ERROR)
  {
    result = os_win32_net_address_from_sockaddr_in(addr);
  }
  return result;
}

static B32
os_socket_connect(OS_Handle socket, OS_NetAddress address)
{
  struct sockaddr_in sockaddrin = os_win32_sockaddr_in_from_net_address(address);
  SOCKET sock = (SOCKET)socket.u64[0];
  S32 connect_result = connect(sock, (struct sockaddr *)&sockaddrin, sizeof(sockaddrin));
  B32 result = connect_result != SOCKET_ERROR;
  return result;
}

static OS_NetAcceptResult
os_socket_accept(OS_Handle socket)
{
  OS_NetAcceptResult result = {};
  SOCKET listen_socket = (SOCKET)socket.u64[0];
  S32 listen_result = listen(listen_socket, SOMAXCONN);
  if(listen_result != SOCKET_ERROR)
  {
    struct sockaddr_in connected_addr = {};
    S32 connected_addrlen = sizeof(connected_addr);
    SOCKET connected_socket = accept(listen_socket, (struct sockaddr *)&connected_addr, &connected_addrlen);
    result.succeeded = connected_socket != INVALID_SOCKET;
    if(connected_socket != INVALID_SOCKET)
    {
      result.address = os_win32_net_address_from_sockaddr_in(connected_addr);
      result.socket.u64[0] = (U64)connected_socket;
    }
  }
  return result;
}

static B32
os_socket_send(OS_Handle socket, String8 data)
{
  SOCKET sock = (SOCKET)socket.u64[0];
  S32 send_result = send(sock, (char *)data.data, safe_s32_from_u64(data.size), 0);
  B32 result = send_result != SOCKET_ERROR;
  return result;
}

static OS_NetReceiveResult
os_socket_receive(Arena *arena, OS_Handle connected_socket, U64 cap)
{
  OS_NetReceiveResult result = {};
  U8 *buffer = push_array_no_zero<U8>(arena, cap);
  SOCKET sock = (SOCKET)connected_socket.u64[0];
  S32 bytes_received = recv(sock, (char *)buffer, safe_s32_from_u64(cap), 0);
  if(bytes_received == SOCKET_ERROR)
  {
    bytes_received = 0;
  }
  result.data.size = safe_u64_from_s32(bytes_received);
  result.data.data = buffer;
  return result;
}

static B32
os_socket_send_to(OS_Handle socket, OS_NetAddress address, String8 data)
{
  SOCKET sock = (SOCKET)socket.u64[0];
  struct sockaddr_in sockaddrin = os_win32_sockaddr_in_from_net_address(address);
  S32 sendto_result = sendto(sock, (char *)data.data, (S32)data.size, 0, (struct sockaddr *)&sockaddrin, sizeof(sockaddrin));
  B32 result = sendto_result != SOCKET_ERROR;
  return result;
}

static OS_NetReceiveResult
os_socket_receive_from(Arena *arena, OS_Handle listen_socket, U64 cap)
{
  OS_NetReceiveResult result = {};
  SOCKET sock = (SOCKET)listen_socket.u64[0];
  struct sockaddr_in sockaddrin = {};
  S32 from_len = sizeof(sockaddrin);
  U8 *buffer = push_array_no_zero<U8>(arena, cap);
  S32 bytes_received = recvfrom(sock, (char *)buffer, safe_s32_from_u64(cap), 0, (struct sockaddr *)&sockaddrin, &from_len);

  if(bytes_received == SOCKET_ERROR)
  {
    bytes_received = 0;
  }
  result.data.data = buffer;
  result.data.size = safe_u64_from_s32(bytes_received);
  result.address = os_win32_net_address_from_sockaddr_in(sockaddrin);
  return result;
}