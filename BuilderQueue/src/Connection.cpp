#include "Connection.h"
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/core/ignore_unused.hpp>

using namespace std::placeholders;

void Connection::wait_for_close() {
    // Persist this connection
    auto self(shared_from_this());

    Logger::info("Waiting for connection to close");
    stream.async_read(buffer, [this, self] (beast::error_code error, std::size_t bytes) {
        boost::ignore_unused(bytes);

        if(error == websocket::error::closed) {
            Logger::info("Cleaning closing connection");
        } else {
            Logger::error("Error closing connection: " + error.message());
        }
    });
}

void Connection::builder_ready(BuilderData builder) {
    // Persist this connection
    auto self(shared_from_this());

    Logger::info("Serializing builder: " + builder.id);
    this->builder = builder;
    std::ostringstream archive_stream;
    boost::archive::text_oarchive archive(archive_stream);
    archive << builder;
    auto serialized_builder = archive_stream.str();
    std::string request_string(serialized_builder);

    Logger::info("Writing builder: " + builder.id);
    stream.async_write(asio::buffer(request_string), [this, self, builder] (beast::error_code error, std::size_t bytes){
        boost::ignore_unused(bytes);
        if(!error) {
            wait_for_close();
        } else {
            Logger::error("Error writing builder: " + builder.id);
        }
    });
}

void Connection::request_builder() {
    // Persist this connection
    auto self(shared_from_this());

    Logger::info("Request to checkout builder made");
    // Pass the provide_builder callback to the queue
    // 'self' is passed to keep the connection alive as long as it's waiting for a builder in the queue
    queue.checkout_builder([this, self] (BuilderData builder) {
        builder_ready(builder);
    });
}

void Connection::read_request_string() {
    // Persist this connection
    auto self(shared_from_this());

    Logger::info("Reading initial request");
    stream.async_read(buffer, [this, self] (beast::error_code error, std::size_t bytes) {
        boost::ignore_unused(bytes);
        if(!error) {
            auto request = beast::buffers_to_string(buffer.data());
            buffer.consume(buffer.size());
            if(request == "checkout_builder_request") {
                request_builder();
            } else {
                Logger::error("Bad initial request string: " + request);
            }
        } else {
            Logger::error("Failed to read initial request string from client: " + error.message());
        }
    });
}

void Connection::start() {
    // Persist this connection
    auto self(shared_from_this());

    // Accept websocket handshake
    Logger::info("waiting for client WebSocket handshake");
    stream.async_accept([this, self](beast::error_code error){
        if(!error) {
            read_request_string();
        } else {
            Logger::error("WebSocket handshake error: " + error.message());
        }
    });
}