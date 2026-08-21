#pragma once

#include "search_server.h"
#include <cstdint>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

class TcpServer {
public:
    TcpServer(uint16_t port, const SearchServer& search_engine);
    ~TcpServer();

    void Run();

private:
    void SetNonBlocking(int sockfd);
    void AcceptConnection();
    void HandleClientData(int client_fd);

    uint16_t port_;
    int server_fd_;
    int epoll_fd_;
    const SearchServer& search_engine_;
};