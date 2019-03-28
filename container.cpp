#include "container.h"

#include <iostream>
#include <thread>
#include <functional>
#include <string>
#include <utility>
#include <cstring>

#include <unistd.h>

namespace judgement {
    Container &Container::instance() {
        static Container container;
        return container;
    }

    const status_t &Container::status(int id) {
        return this->judges.at(id).get_status();
    }

    void Container::handle(zmq::context_t &context) {
        zmq::socket_t socket(context, ZMQ_REP);
        socket.connect(INPROC_ADDRESS);
        while (true) {
            zmq::message_t request;
            socket.recv(&request);
            std::cout << "Received <" << request << ">" << std::endl;
            if (!request.size())
                return;
            std::thread(&Container::run, this, std::cref(request), std::ref(socket)).join();
        }
    }

    void Container::run(const zmq::message_t &request, zmq::socket_t &socket) {
        std::string str = std::string(static_cast<const char *>(request.data()), request.size());
        if (str.empty() || str == " ")
            return;
        input_t input = split(str);
        this->judges.insert(std::make_pair(input.id, Judge({input.name, input.ext, input.source_type})));
        this->judges.at(input.id).run();
        std::string message = status_message(this->status(input.id));
        std::cout << std::to_string(input.id) + " " + message << std::endl;
        zmq::message_t reply(message.size());
        memcpy(reply.data(), message.data(), message.size());
        socket.send(reply);
        this->judges.erase(input.id);
    }
}