#include "coro_async.hpp"
#include <iostream>
#include <thread>
#include <memory>

using namespace coro_async;

class handler: public std::enable_shared_from_this<handler> {
public:
    handler(stream_socket client)
        : client_{std::move(client)}
    {}

    handler(const handler&) = delete;

    handler& operator=(const handler&) = delete;

    ~handler() {
        client_.close();
    }

    void start() {
        do_read();
    }

private:
    void do_read() {
        auto self{shared_from_this()};
        auto rdbuf = buffer::buffer_ref{buf_, buf_size_};
        client_.async_read_some(rdbuf, [this, self](const std::error_code ec, const size_t rd_bytes) {
            if (!ec) {
                do_write(rd_bytes);
            }
        });        
    }

    void do_write(const size_t wr_bytes) {
        auto self{shared_from_this()};
        auto wrbuf = buffer::buffer_ref{buf_, wr_bytes};
        client_.async_write(wrbuf, [this, self](const std::error_code ec, const size_t wr_bytes) {
            if (ec) {
                do_read();
            }
        });
    }

private:
    static constexpr int buf_size_ = 1024;
    char buf_[buf_size_];
    stream_socket client_;
};

void server_run(tcp_acceptor &acc) {
    while (true) {
        stream_socket client_sock{acc.get_io_service()};
        acc.async_accept(client_sock, [&client_sock](const std::error_code ec) {
            if (ec) {
                std::cerr << "Accept failed: " << ec.message() << '\n';
            }
            std::clog << "New Client connected with fd: " << client_sock.get_native_handle() << "\n";
            std::make_shared<handler>(std::move(client_sock))->start();
          });
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Usage: coro_server [port]\n";
        return 1;
    }
    int port = std::stoi(argv[1]); 
    if (port <= 0 || port >= 65536) {
        std::cerr << "Bad port number\n";
        return 1;
    }
    io_service ios{};
    tcp_acceptor acceptor{ios};
    const int backlog = 1000;

    std::error_code ec{};
    std::clog << "Starting server on 127.0.0.1:" << port << '\n';
    acceptor.open(ec);
    if (ec) {
        std::cerr << "Open server port failed: " << ec.message() << '\n';
        return 1;
    }

    endpoint listen_addr = endpoint{v4_address{"127.0.0.1"}, static_cast<unsigned short>(port)};
    acceptor.bind(listen_addr, ec);
    if (ec) {
        std::cerr << "Open server port failed: " << ec.message() << '\n';
        return 1;
    }

    acceptor.listen(backlog, ec);
    if (ec) {
        std::cerr << "Open server port failed: " << ec.message() << '\n';
        return 1;
    }

    std::thread thr{[&] { ios.run(); }};
    server_run(acceptor);
    thr.join();

    return 0;
}