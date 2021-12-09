#include "coro_async.hpp"
#include <iostream>
#include <thread>

using namespace coro_async;

coro_task_auto<void> handle_client(coro_socket client) {
    const int buf_size = 1024;
    char buf[buf_size];
    for (;;) {
        auto rbref = as_buffer(buf);
        auto rd_bytes = co_await client.read(buf_size, rbref);
         if (rd_bytes.is_error()) {
            if (rd_bytes.error().value() == 3) { // would block error code
                continue;
            }
            break;
        }
        auto wbref = buffer::buffer_ref{buf, rd_bytes.result()};
        co_await client.write(rd_bytes.result(), wbref);
    }
    client.close();
    co_return;
}

coro_task_auto<void> server_run(coro_acceptor &acc) {
    while (true) {
        auto result = co_await acc.accept();
        if (result.is_error()) {
            std::cerr << "Accept failed: " << result.error().message() << '\n';
            co_return;
        }
        handle_client(std::move(result.result()));
    }
    co_return;
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
    coro_acceptor acceptor{ios};

    std::error_code ec{};
    std::clog << "Starting server on 127.0.0.1:" << port << '\n';
    acceptor.open("127.0.0.1", port, ec);
    if (ec) {
        std::cerr << "Open server port failed: " << ec.message() << '\n';
        return 1;
    }

    server_run(acceptor);
    ios.run();
    // std::thread thr{[&] { ios.run(); }};
    // thr.join();
    return 0;
}