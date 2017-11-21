#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include "Messenger.h"

namespace asio = boost::asio;
using asio::ip::tcp;

void create_def() {
    std::cout << "creating simple definition file\n";
    std::ofstream outfile("container.def");
    outfile << "BootStrap: docker\nFrom: ubuntu:zesty" << std::endl;
    outfile.close();
}

int main(int argc, char *argv[]) {
    try {
        asio::io_service io_service;
        tcp::socket socket(io_service);
        tcp::resolver resolver(io_service);
        asio::connect(socket, resolver.resolve({std::string("127.0.0.1"), std::string("8080")}));

        Messenger messenger(socket);

        // Initiate a build request
        messenger.send("build_request");

        create_def();

        // Send the definition file
        messenger.send_file("./container.def");

        // Read the build output until a zero length message is sent
        std::string line;
        do {
            line = messenger.receive();
            std::cout << line;
        } while (!line.empty());

        // Read the container image
        messenger.receive_file("./container.img");

        std::cout << "Container built!\n";

    }
    catch (std::exception &e) {
        std::cerr << "\033[1;31m Failed to build container: " << e.what() << "\033[0m\n";
    }

    return 0;
}