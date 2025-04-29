#include "websocket_client.hpp"
#include <chrono>
#include <iostream>

WebSocketClient::WebSocketClient() : running_( false ) {}

WebSocketClient::~WebSocketClient()
{
    stop();
}

void WebSocketClient::setHost( const std::string& host )
{
    host_ = host;
}

void WebSocketClient::setPort( const std::string& port )
{
    port_ = port;
}

void WebSocketClient::start()
{
    if ( ! running_ )
    {
        running_      = true;
        clientThread_ = std::thread( &WebSocketClient::connectAndRun, this );
    }
}

void WebSocketClient::stop()
{
    if ( running_ )
    {
        running_ = false;
        if ( clientThread_.joinable() )
        {
            ws_->close( websocket::close_code::normal );
            // clientThread_.join();
        }
    }
}

void WebSocketClient::connectAndRun()
{
    try
    {
        net::io_context ioc;
        tcp::resolver   resolver( ioc );
        ws_ = new websocket::stream< tcp::socket >( ioc );
        //
        auto const results = resolver.resolve( host_, port_ );
        net::connect( ws_->next_layer(), results.begin(), results.end() );
        ws_->handshake( host_, "/" );
        //
        std::thread receiveThread( &WebSocketClient::handleReceive, this );
        receiveThread.join();
    }
    catch ( const std::exception& e )
    {
        std::cerr << "Client exception: " << e.what() << std::endl;
    }
}

void WebSocketClient::handleReceive()
{
    try
    {
        beast::flat_buffer buffer;
        while ( running_ )
        {
            ws_->read( buffer );
            std::string message = beast::buffers_to_string( buffer.data() );
            std::cout << "Client received: " << message << std::endl;
            //
            if ( message == "stop" )
            {
                stop();
                //
                break;
            }
            //
            buffer.consume( buffer.size() );
        }
    }
    catch ( const std::exception& e )
    {
        std::cerr << "Client receive exception: " << e.what() << std::endl;
    }
}

void WebSocketClient::handleSend( std::string text )
{
    if ( running_ )
    {
        try
        {
            ws_->write( net::buffer( text ) );
        }
        catch ( ... )
        {
            //
        }
    }
}