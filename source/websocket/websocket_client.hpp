#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <string>
#include <thread>

namespace beast     = boost::beast;
namespace http      = beast::http;
namespace websocket = beast::websocket;
namespace net       = boost::asio;
using tcp           = net::ip::tcp;

class WebSocketClient
{
public:
    WebSocketClient();
    ~WebSocketClient();
    void setHost( const std::string& host );
    void setPort( const std::string& port );
    void start();
    void stop();
public:
    void handleReceive();
    void handleSend(std::string text);
private:
    void connectAndRun();
private:
    std::string                       host_;
    std::string                       port_;
    std::thread                       clientThread_;
    bool                              running_;
    websocket::stream< tcp::socket >* ws_;
};

#endif