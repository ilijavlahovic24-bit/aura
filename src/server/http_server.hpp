#ifndef AURA_SERVER_HTTP_SERVER_HPP
#define AURA_SERVER_HTTP_SERVER_HPP

#include <functional>
#include <string>
#include <atomic>

namespace aura::server {

using Handler = std::function<std::string(const std::string& path)>;

class HttpServer {
public:
    HttpServer(int port, Handler h);
    ~HttpServer();
    void run();      // blocking
    void stop();

private:
    int port_;
    Handler handler_;
    int listen_fd_ = -1;
    std::atomic<bool> running_{true};
    void handle_client(int fd);
};

} // namespace aura::server

#endif