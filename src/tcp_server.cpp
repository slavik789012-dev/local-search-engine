#include "tcp_server.h"
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>

TcpServer::TcpServer(uint16_t port, const SearchServer& search_engine) : port_(port), search_engine_(search_engine)
{
	server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd_ == -1) {
		throw std::runtime_error("Failed to create socket");
	}
	int opt = 1;
	if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		throw std::runtime_error("Failed to set SO_REUSEADDR");
	}
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port_);
	if (bind(server_fd_, (const sockaddr*)&address, sizeof(address)) == -1) {
		throw std::runtime_error("Failed to bind to port");
	}
	if (listen(server_fd_, SOMAXCONN) == -1) {
		throw std::runtime_error("Failed to listen");
	}
	epoll_fd_ = epoll_create(1024);
	if (epoll_fd_ == -1) {
		throw std::runtime_error(std::string("Failed to create epoll instance: ") + std::strerror(errno));
	}
	struct epoll_event events;
	events.events = EPOLLIN;
	events.data.fd = server_fd_;
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_fd_, &events) == -1) {
		throw std::runtime_error("Failed to add server_fd to epoll");
	}
}

TcpServer::~TcpServer()
{
	if (server_fd_ != -1) close(server_fd_);
	if (epoll_fd_ != -1) close(epoll_fd_);
}

constexpr int MAX_EVENTS = 64;
constexpr int BUFFER_SIZE = 4096;

void TcpServer::Run()
{
	struct epoll_event events[MAX_EVENTS];
	std::cout << "Search engine server is running and listening on port " << port_ << "...\n";
	while (true) {
		int num_events = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
		if (num_events == -1) {
			std::cerr << "epoll_wait failed\n";
			break;
		}
		for (int i = 0; i < num_events; ++i) {
			if (events[i].data.fd == server_fd_) {
				AcceptConnection();
			}
			else {
				HandleClientData(events[i].data.fd);
			}
		}
	}
}

void TcpServer::SetNonBlocking(int sockfd)
{
	int flags = fcntl(sockfd, F_GETFL, 0);
	if (flags == -1) {
		throw std::runtime_error("fcntl F_GETFL failed");
	}
	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) throw std::runtime_error("fcntl F_SETFL failed");
}

void TcpServer::AcceptConnection()
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd == -1) {
		std::cerr << "Failed to accept client connection\n";
		return;
	}
	SetNonBlocking(client_fd);
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = client_fd;
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &event) == -1) {
		std::cerr << "Failed to add client to epoll\n";
		close(client_fd);
	}
	else {
		std::cout << "[Server] New client connected. FD: " << client_fd << "\n";
	}
}

void TcpServer::HandleClientData(int client_fd) {
	char buffer[BUFFER_SIZE];
	std::string query;
	while (true) {
		ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);
		if (bytes_read == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break;
			}
			else {
				std::cerr << "[Server] recv error on FD " << client_fd << "\n";
				close(client_fd);
				return;
			}
		}
		else if (bytes_read == 0) {
			std::cout << "[Server] Client disconnected. FD: " << client_fd << "\n";
			close(client_fd);
			return;
		}
		else {
			query.append(buffer, bytes_read);
		}
	}

	while (!query.empty() && (query.back() == '\n' || query.back() == '\r')) {
		query.pop_back();
	}

	if (query.empty()) return;

	std::cout << "[Server] Received query from FD " << client_fd << ": {" << query << "}\n";

	auto results = search_engine_.search(query);

	std::string response;
	if (results.empty()) {
		response = "No matches found.\n";
	}
	else {
		response = "Found documents (Top-" + std::to_string(results.size()) + "):\n";
		for (const auto& doc : results) {
			response += "  DocID: " + std::to_string(doc.id) +
				" (Relevance: " + std::to_string(doc.relevance) + ")\n";
		}
	}
	response += "-----------------------\n";

	ssize_t bytes_sent = send(client_fd, response.c_str(), response.size(), 0);
	if (bytes_sent == -1) {
		std::cerr << "[Server] send error on FD " << client_fd << "\n";
	}
}

