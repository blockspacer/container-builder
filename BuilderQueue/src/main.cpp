#include "BuilderQueue.h"
#include "Server.h"

namespace asio = boost::asio;

int main(int argc, char *argv[]) {
    asio::io_context io_context;

    BuilderQueue queue(io_context);
    Server server(io_context, queue);

    // Keep running even in the event of an exception
    for (;;) {
        try {
            io_context.run();
        } catch (...) {
            SEVERE_LOG("Unknown io_service exception during run");
        }
    }
}