#include <iostream>
#include <thread>
#include "boost/asio.hpp"

#include "src/HttpPublisher.hpp"

using namespace std;
using namespace boost::asio;

io_service io(1);
HttpPublisher publisher(io);

void publishMessage(string host, int port, string url, string message) {
    publisher.publish(move(host), port, concatenate(
            "POST ", url, " HTTP/1.0\n"
                    "Host: ", host, "\n"
                    "Content-type: application/x-www-form-urlencoded\n"
                    "Content-length: ", lexical_cast<string>(message.length()),
            "\nConnection: keep-alive"
                    "\n"
                    "\n",
            message
    ));
}

int main() {
    int i = 0;
    thread([&](){
        while(true) {
            if(i != 0)
                cout << i << endl;
            i = 0;
            usleep(1*1000*1000);
        }
    }).detach();

    boost::asio::io_service::work work(io);
    thread ([&]() {
        for (int f = 0; f < 100000; ++f) {
            io.post([f, &publisher]() {
                publishMessage("127.0.0.1", 90, "/pub?id=5", lexical_cast<string>(f));
            });
            usleep(1000*1000);
        }
    }).detach();
    io.run();
    return 0;
}
