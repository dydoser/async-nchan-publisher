#ifndef ASYNC_NCHAN_PUBLISHER_NCHANPUBLISHER_HPP
#define ASYNC_NCHAN_PUBLISHER_NCHANPUBLISHER_HPP

#include <iostream>
#include <map>
#include <boost/asio.hpp>

#include "PublisherConnection.hpp"

using namespace std;

class HttpPublisher {
    boost::asio::io_service &io;
public:
    map<string, PublisherConnection*> connections;

    HttpPublisher(boost::asio::io_service &service) : io(service) {

    };

    void publish(string host, int port,string request) {
        string key = host;
        key += ":";
        key += lexical_cast<string>(port);
        auto connIt = connections.find(key);
        if (connIt == connections.end())
            connIt = connections.insert(make_pair(key, new PublisherConnection(io, host, port))).first;
        connIt->second->publish(request);
    }

    void disconnect() {
        for(auto conn: connections) {
            delete conn.second;
        }
        connections.clear();
    }

    ~HttpPublisher() {
        for(auto conn: connections) {
             delete conn.second;
        }
    }
};


#endif //ASYNC_NCHAN_PUBLISHER_NCHANPUBLISHER_HPP
