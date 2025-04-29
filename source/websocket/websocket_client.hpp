#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <thread>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class WebSocketClient {
public:
    WebSocketClient();
    ~WebSocketClient();
    void setHost(const std::string& host);
    void setPort(const std::string& port);
    void start();
    void stop();

private:
    void connectAndRun();
    void sendAndReceiveMessages(websocket::stream<tcp::socket>& ws);

    std::string host_;
    std::string port_;
    std::thread clientThread_;
    bool running_;
};

#endif