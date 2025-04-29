#include "websocket_client.hpp"
#include <iostream>
#include <chrono>

WebSocketClient::WebSocketClient() : running_(false) {}

WebSocketClient::~WebSocketClient() {
    stop();
}

void WebSocketClient::setHost(const std::string& host) {
    host_ = host;
}

void WebSocketClient::setPort(const std::string& port) {
    port_ = port;
}

void WebSocketClient::start() {
    if (!running_) {
        running_ = true;
        clientThread_ = std::thread(&WebSocketClient::connectAndRun, this);
    }
}

void WebSocketClient::stop() {
    if (running_) {
        running_ = false;
        if (clientThread_.joinable()) {
            clientThread_.join();
        }
    }
}

void WebSocketClient::connectAndRun() {
    try {
        net::io_context ioc;
        tcp::resolver resolver(ioc);
        websocket::stream<tcp::socket> ws(ioc);

        auto const results = resolver.resolve(host_, port_);
        net::connect(ws.next_layer(), results.begin(), results.end());
        ws.handshake(host_, "/");

        sendAndReceiveMessages(ws);

        ws.close(websocket::close_code::normal);
    } catch (const std::exception& e) {
        std::cerr << "Client exception: " << e.what() << std::endl;
    }
}

void WebSocketClient::sendAndReceiveMessages(websocket::stream<tcp::socket>& ws) {
    beast::flat_buffer buffer;
    while (running_) {
        try {
            // 发送消息
            ws.write(net::buffer("Periodic message from client"));
            std::this_thread::sleep_for(std::chrono::seconds(5));

            // 接收消息
            ws.read(buffer);
            std::string message = beast::buffers_to_string(buffer.data());
            std::cout << "Client received: " << message << std::endl;
            buffer.consume(buffer.size());
        } catch (...) {}
    }
}