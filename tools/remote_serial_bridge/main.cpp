#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <ws2tcpip.h>
#else
#include <cstring>
//#include <stdlib.h>
//#include <string.h>
#include <unistd.h>
//#include <sys/types.h> 
#include <sys/socket.h>
//#include <netinet/in.h>
#include <netdb.h> 
#endif
#include <iostream>
#include <thread>
#include "console_log.h"
#include "serial_stream.h"

bool app_exiting{ false };

#define SERIAL_BUFFER_SIZE 2048
#define SOCKET_BUFFER_SIZE 2048

static uint8_t serial_buffer[SERIAL_BUFFER_SIZE];
static uint8_t socket_buffer[SOCKET_BUFFER_SIZE];

#if _WIN32
  using TcpSocket = SOCKET;
#else
  using TcpSocket = int;
  constexpr int INVALID_SOCKET=-1;
  constexpr int SOCKET_ERROR=-1;
#endif

#if _WIN32
TcpSocket open_socket(char* addr, unsigned int port) {
  WSADATA wsaData;
  TcpSocket socket_res = INVALID_SOCKET;
  struct addrinfo* result = NULL,
    * ptr = NULL,
    hints;
  int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (res != 0) {
    HAIER_LOGE("WSAStartup failed with error: %d", res);
    return INVALID_SOCKET;
  }
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  // Resolve the server address and port
  res = getaddrinfo(addr, std::to_string(port).c_str(), &hints, &result);
  if (res != 0) {
    HAIER_LOGE("getaddrinfo failed with error: %d", res);
    WSACleanup();
    return INVALID_SOCKET;
  }
  for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
    // Create a TcpSocket for connecting to server
    socket_res = socket(ptr->ai_family, ptr->ai_socktype,
      ptr->ai_protocol);
    if (socket_res == INVALID_SOCKET) {
      HAIER_LOGE("socket failed with error: %ld", WSAGetLastError());
      WSACleanup();
      return INVALID_SOCKET;
    }
    // Connect to server.
    res = connect(socket_res, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (res == SOCKET_ERROR) {
      closesocket(socket_res);
      socket_res = INVALID_SOCKET;
      continue;
    }
    break;
  }
  freeaddrinfo(result);
  return socket_res;
}

void close_socket(TcpSocket& socket) {
  if (socket != INVALID_SOCKET)
    closesocket(socket);
  WSACleanup();
}

int read_socket(TcpSocket& socket, uint8_t *buffer, size_t buffer_size) {
  unsigned long size;
  ioctlsocket(socket, FIONREAD, &size);
  if (size > 0) {
    int res = recv(socket, (char*)socket_buffer, SOCKET_BUFFER_SIZE, 0);
    if (res > 0)
      return res;
  }
  return 0;
}

int write_socket(TcpSocket& socket, uint8_t* buffer, size_t buffer_size) {
  return send(socket, (char*)buffer, buffer_size, 0);
}

int get_last_error() {
  return WSAGetLastError();
}

#else
TcpSocket open_socket(char* addr, unsigned int port) {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    return INVALID_SOCKET;
  struct hostent *server;
  server = gethostbyname(addr);
  if (server == NULL)
    return INVALID_SOCKET;
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
  if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) 
    return INVALID_SOCKET;
  return sockfd;
}

void close_socket(TcpSocket srv_socket) {
  close(srv_socket);
}

int read_socket(TcpSocket& socket, uint8_t* buffer, size_t buffer_size) {
  return read(socket, buffer, buffer_size);
}

int write_socket(TcpSocket& socket, uint8_t* buffer, size_t buffer_size) {
  return write(socket, buffer, buffer_size);
}

int get_last_error() {
  return 0;
}

#endif

int main(int argc, char** argv) {
  haier_protocol::set_log_handler(console_logger);
  if (argc == 3) {
    HAIER_LOGI("Opening socket");
    TcpSocket remote_socket = open_socket(argv[2], 8888);
    if (remote_socket == INVALID_SOCKET) {
      HAIER_LOGE("Failed to connect to server %s", argv[2]);
      close_socket(remote_socket);
      return 1;
    }
    HAIER_LOGI("Opening port %s", argv[1]);
    haier_protocol::set_log_handler(console_logger);
    SerialStream serial_stream(argv[1]);
    if (!serial_stream.is_valid()) {
      HAIER_LOGE("Can't open port %s", argv[1]);
      return 1;
    }
    int res;
    while (!app_exiting) {
      int kb = get_kb_hit();
      if (kb != NO_KB_HIT) {
        if (kb == 27)
          app_exiting = true;
      } else {
        unsigned long size = serial_stream.available();
        size = serial_stream.read_array(serial_buffer, size);
        if (size > 0) {
          HAIER_BUFD("SERIAL>>", serial_buffer, size);
          res = write_socket(remote_socket, serial_buffer, size);
          if (res < 0) {
            HAIER_LOGE("send failed with error: %d", get_last_error());
            close_socket(remote_socket);
            return 1;
          }
        }
        res = read_socket(remote_socket, socket_buffer, SOCKET_BUFFER_SIZE);
        if (res > 0) {
          HAIER_BUFD("SOCKET>>", socket_buffer, res);
          serial_stream.write_array(socket_buffer, res);
        } else if (res < 0) {
          HAIER_LOGE("recv failed with error: %d", get_last_error());
          close_socket(remote_socket);
          return 1;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }
    close_socket(remote_socket);
  } else {
    HAIER_LOGE("Please use: %s <remote_port> <local_port>", argv[0]);
  }
}
