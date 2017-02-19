#ifndef ASYNC_NCHAN_PUBLISHER_NCHANCONNECTION_HPP
#define ASYNC_NCHAN_PUBLISHER_NCHANCONNECTION_HPP

#include <iostream>
#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "boost/lexical_cast.hpp"
#include "../../../helpers/Concatenate.hpp"

using namespace std;
using namespace boost::asio::ip;

class PublisherConnection {
    tcp::resolver mResolver;
    boost::asio::io_service &io;
    string* mSendBuffer;
    char buf[65536];
    boost::asio::ip::tcp::socket mSocket;
    bool sendScheduled = false;
    string host;
    int port;
    bool connected = false;

    void mStartCommunication() {
        if(mSendBuffer)
            delete(mSendBuffer);
        mSendBuffer = new string();
        tcp::resolver::query query(host, lexical_cast<string>(port),tcp::resolver::query::canonical_name);
        mResolver.async_resolve(query,
                                boost::bind(&PublisherConnection::mHandleResolve, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::iterator));
    }

    void mHandleReceive() {
        mSocket.async_read_some(boost::asio::buffer(buf, 65536),
                          [&](boost::system::error_code error, std::size_t bytes_transferred){
                              if(error) {
                                  return;
                              }
                              mHandleReceive();
                          });
    }

    void mHandleConnect(const boost::system::error_code &err,
                        tcp::resolver::iterator endpoint_iterator)
    {
        if (err && (endpoint_iterator != tcp::resolver::iterator()))
        {
            std::cout << "Error: " << err.message() << "\n";
            mSocket.close();

            tcp::endpoint endpoint = *endpoint_iterator;
            mSocket.async_connect(endpoint,
                                  boost::bind(&PublisherConnection::mHandleConnect, this,
                                              boost::asio::placeholders::error, ++endpoint_iterator));
        }
        if(!err) {
            if(!sendScheduled && !mSendBuffer->empty()) {
                mSendQueue();
            }
            mHandleReceive();
            connected = true;
            cout << "connected" << endl;
        }
    }

    void mHandleResolve(const boost::system::error_code &err,
                        tcp::resolver::iterator endpoint_iterator)
    {
        if (!err)
        {
            tcp::endpoint endpoint = *endpoint_iterator;
            mSocket.async_connect(endpoint,
                                  boost::bind(&PublisherConnection::mHandleConnect, this,
                                              boost::asio::placeholders::error, ++endpoint_iterator));
        }
        else
        {
            std::cout << "Error: " << err.message() << "\n";
        }
    }

    void mSendQueue() {
        sendScheduled = true;
        string* str = mSendBuffer;
        mSendBuffer = new string();
        boost::asio::async_write(mSocket, boost::asio::buffer(str->c_str(), str->length()),
            [str, this](boost::system::error_code err, std::size_t size){
                delete str;
                if(err)
                {
                    sendScheduled = false;
                    mSocket.close();
                    cout << "err!" << err.message() << endl;
                    mStartCommunication();
                    //TODO: reconect
                    return;
                }
                cout << "sent!" << endl;
                if(!mSendBuffer->empty()) {
                    mSendQueue();
                }
                else {
                    sendScheduled = false;
                }
            }
        );
    }
public:
    PublisherConnection (boost::asio::io_service &service, string& host, int port) :
            io(service), mSocket(service), host(host), port(port), mResolver(service) {
        mStartCommunication();
    };

    void publish(string& request) {
        *mSendBuffer += request;
        if(!sendScheduled && connected) {
            mSendQueue();
        }
    }
};

#endif //ASYNC_NCHAN_PUBLISHER_NCHANCONNECTION_HPP