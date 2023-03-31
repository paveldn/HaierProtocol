#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "console_log.h"
#include <conio.h>
#include <thread>


#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
bool app_exiting{ false };

#define SERIAL_BUFFER_SIZE 2048
#define SOCKET_BUFFER_SIZE 2048

static uint8_t serial_buffer[SERIAL_BUFFER_SIZE];
static uint8_t socket_buffer[SOCKET_BUFFER_SIZE];

SOCKET open_socket(char* addr, char* port) {
  WSADATA wsaData;
  SOCKET socket_res = INVALID_SOCKET;
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
  res = getaddrinfo(addr, port, &hints, &result);
  if (res != 0) {
    HAIER_LOGE("getaddrinfo failed with error: %d", res);
    WSACleanup();
    return INVALID_SOCKET;
  }
  for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
    // Create a SOCKET for connecting to server
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

void close_socket(SOCKET srv_socket) {
  if (srv_socket != INVALID_SOCKET)
    closesocket(srv_socket);
  WSACleanup();
}

HANDLE open_serial(char* port_name) {
  HANDLE handle_ = CreateFile(std::string("\\\\.\\").append(port_name).c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  DCB serialParams = { 0 };
  serialParams.DCBlength = sizeof(serialParams);
  GetCommState(handle_, &serialParams);
  serialParams.BaudRate = 9600;
  serialParams.ByteSize = 8;
  serialParams.StopBits = 1;
  serialParams.fBinary = 1;
  serialParams.fRtsControl = 0;
  serialParams.fDtrControl = 0;
  serialParams.XonLim = 2048;
  serialParams.XoffLim = 512;
  serialParams.Parity = 0;
  serialParams.XonChar = 17;
  serialParams.XoffChar = 19;
  serialParams.EofChar = 26;
  SetCommState(handle_, &serialParams);
  COMMTIMEOUTS timeout = { 0 };
  timeout.ReadIntervalTimeout = MAXDWORD;
  timeout.ReadTotalTimeoutConstant = 0;
  timeout.ReadTotalTimeoutMultiplier = 0;
  timeout.WriteTotalTimeoutConstant = 2;
  timeout.WriteTotalTimeoutMultiplier = 0;
  SetCommTimeouts(handle_, &timeout);
  return handle_;
}

int main(int argc, char** argv) {
  haier_protocol::set_log_handler(console_logger);
  if (argc == 3) {
    HAIER_LOGI("Opening socket");
    SOCKET remote_socket = open_socket(argv[2], "8888");
    if (remote_socket == INVALID_SOCKET) {
      HAIER_LOGE("Failed to connect to server %s", argv[2]);
      close_socket(remote_socket);
      return 1;
    }
    HAIER_LOGI("Opening port %s", argv[1]);
    HANDLE handle_ = open_serial(argv[1]);
    int res;
    while (!app_exiting) {
      if (kbhit()) {
        char ch = getch();
        if (ch == 27)
          app_exiting = true;
      } else {
        unsigned long size;
        int status = ReadFile(handle_, serial_buffer, SERIAL_BUFFER_SIZE, &size, nullptr);
        if ((status != 0) && (size > 0)) {
          HAIER_BUFD("SERIAL>>", serial_buffer, size);
          res = send(remote_socket, (char*) serial_buffer, size, 0);
          if (res == SOCKET_ERROR) {
            HAIER_LOGE("send failed with error: %d", WSAGetLastError());
            close_socket(remote_socket);
            CloseHandle(handle_);
            return 1;
          }
        }
        unsigned long l;
        ioctlsocket(remote_socket, FIONREAD, &l);
        if (l > 0) {
          res = recv(remote_socket, (char*)socket_buffer, SOCKET_BUFFER_SIZE, 0);
          if (res > 0) {
            HAIER_BUFD("SOCKET>>", socket_buffer, res);
            WriteFile(handle_, socket_buffer, res, nullptr, nullptr);
          } else if (res < 0) {
            HAIER_LOGE("recv failed with error: %d", WSAGetLastError());
            close_socket(remote_socket);
            CloseHandle(handle_);
            return 1;
          }
        }
        Sleep(10);
      }
    }
    CloseHandle(handle_);
    close_socket(remote_socket);
  } else {
    std::cout << "Please use: " << argv[0] << " <remote_port> <local_port>" << std::endl;
  }
}
