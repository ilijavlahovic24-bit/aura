#include "server/http_server.hpp"
#include <cstring>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>

namespace aura::server {

HttpServer::HttpServer(int port, Handler h) : port_(port), handler_(std::move(h)) {}
HttpServer::~HttpServer() { if (listen_fd_ >= 0) ::close(listen_fd_); }

void HttpServer::run() {
    listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) { std::perror("socket"); return; }
    int opt = 1;
    ::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port_);
    if (::bind(listen_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::perror("bind"); return;
    }
    if (::listen(listen_fd_, 16) < 0) { std::perror("listen"); return; }

    std::cerr << "aura serve: http://localhost:" << port_ << "\n";

    while (running_) {
        int fd = ::accept(listen_fd_, nullptr, nullptr);
        if (fd < 0) { if (!running_) break; continue; }
        std::thread([this, fd] { handle_client(fd); ::close(fd); }).detach();
    }
}

void HttpServer::stop() { running_ = false; }

void HttpServer::handle_client(int fd) {
    char buf[4096];
    ssize_t n = ::read(fd, buf, sizeof(buf) - 1);
    if (n <= 0) return;
    buf[n] = 0;

    std::string req(buf);
    auto sp1 = req.find(' ');
    auto sp2 = req.find(' ', sp1 + 1);
    if (sp1 == std::string::npos || sp2 == std::string::npos) return;
    std::string method = req.substr(0, sp1);
    std::string path   = req.substr(sp1 + 1, sp2 - sp1 - 1);
    if (method != "GET") {
        std::string resp = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
        (void)::write(fd, resp.data(), resp.size());
        return;
    }

    std::string body;
    std::string status = "200 OK";
    std::string content_type = "text/html; charset=utf-8";
    try {
        body = handler_(path);
        if (body.empty()) {
            status = "404 Not Found";
            body = "not found";
            content_type = "text/plain";
        }
    } catch (const std::exception& e) {
        status = "500 Internal Server Error";
        body = std::string("error: ") + e.what();
        content_type = "text/plain";
    }

    std::ostringstream resp;
    resp << "HTTP/1.1 " << status << "\r\n"
         << "Content-Type: " << content_type << "\r\n"
         << "Content-Length: " << body.size() << "\r\n"
         << "Access-Control-Allow-Origin: *\r\n"
         << "Connection: close\r\n\r\n"
         << body;
    auto s = resp.str();
    (void)::write(fd, s.data(), s.size());
}

} // namespace aura::server