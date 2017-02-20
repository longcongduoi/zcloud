/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ZApiHTTPClient.cpp
 * Author: tamvh
 * 
 * Created on February 6, 2017, 2:39 PM
 */

#include "ZApiHTTPClient.h"
using namespace Poco;
using namespace Poco::Net;

class ZApiHTTPClient::Impl {
public:
    std::string host;
    int16_t port;
    std::string method;
    std::string api;
    std::string data;
    std::string param;
    std::string reqBody;
};

ZApiHTTPClient::ZApiHTTPClient() : d_ptr(new Impl) {
}

ZApiHTTPClient::~ZApiHTTPClient() {
}

void ZApiHTTPClient::setHost(const std::string host) {
    d_ptr->host = host;
}

void ZApiHTTPClient::setPort(int16_t port) {
    d_ptr->port = port;
}

void ZApiHTTPClient::setMethod(const std::string method) {
    d_ptr->method = method;
}

void ZApiHTTPClient::setApi(const std::string api) {
    d_ptr->api = api;
}

void ZApiHTTPClient::setParam(const std::string key, const std::string value) {
    if(!d_ptr->param.empty()) {
        d_ptr->param.append(",");
    }
    
        d_ptr->param.append("\"" + key + "\"");
        d_ptr->param.append(":");
        d_ptr->param.append("\"" + value + "\"");
       
}

void ZApiHTTPClient::setReqBody() {
//    d_ptr->reqBody = "{" + d_ptr->param + "}";
    d_ptr->reqBody = "[{\"name\": \"iot\", \"timestamp\": 1487090609, \"value\": 1.3}]";
}

void ZApiHTTPClient::sendRequest() {
    try {
//        std::string _uri = "http://" + d_ptr->host + ":" + std::to_string(d_ptr->port) + "/" + d_ptr->api;
        std::string _uri = "http://" + d_ptr->host + "/" + d_ptr->api;
        // prepare session
        URI uri(_uri);
        HTTPClientSession session(uri.getHost(), uri.getPort());
        // prepare path
        std::string path(uri.getPathAndQuery());
        if (path.empty())
            path = "/";
        // send request
        if (d_ptr->method == "GET") {
            HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
            session.sendRequest(req);
        } else if (d_ptr->method == "POST") {
            HTTPRequest req(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_0);
            session.sendRequest(req) << d_ptr->reqBody;
        }

        // get response
        HTTPResponse res;
        // print response
        std::istream &is = session.receiveResponse(res);
        std::string out_str = std::string(std::istreambuf_iterator<char>(is),{});
        d_ptr->data = out_str;
    } catch (std::exception ex) {
        std::cerr << ex.what() << std::endl;
    }
}

std::string ZApiHTTPClient::responseData() {
    return d_ptr->data;
}



