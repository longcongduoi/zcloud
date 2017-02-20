/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ClientWorker.cpp
 * Author: tamvh
 * 
 * Created on February 20, 2017, 4:00 PM
 */

#include "zapihttp.client/ClientWorker.h"
using namespace Poco;
using namespace Net;
using namespace std;
class ClientWorker::Impl {
public:
    std::string method;
    std::string data;
    std::string param;
    std::string reqBody;
    std::string path;
    std::string res_data;
    HTTPClientSession *session;
    HTTPResponse *response;
    HTTPRequest *request;
};

ClientWorker::ClientWorker(const std::string& server,
        const std::string& api) : d_ptr(new Impl) {
    URI uri("http://" + server + "/" + api);
    d_ptr->path = uri.getPathAndQuery();
    if (d_ptr->path.empty()) {
        d_ptr->path = "/";
    }
    d_ptr->session = new HTTPClientSession(uri.getHost(), uri.getPort());
}

ClientWorker::~ClientWorker() {
}

void ClientWorker::setMethod(const std::string method) {
    d_ptr->method = method;
}

void ClientWorker::setParam(
        const std::string key,
        const std::string value) {
    if (!d_ptr->param.empty()) {
        d_ptr->param.append(",");
    }
    d_ptr->param.append("\"" + key + "\"");
    d_ptr->param.append(":");
    d_ptr->param.append("\"" + value + "\"");
}

void ClientWorker::sendRequest() {
    if (d_ptr->method == "POST") {
        //do post
        d_ptr->request = new HTTPRequest(HTTPRequest::HTTP_POST, d_ptr->path, HTTPMessage::HTTP_1_1);
        this->doPost();
    } else if (d_ptr->method == "GET") {
        //do get
        d_ptr->request = new HTTPRequest(HTTPRequest::HTTP_GET, d_ptr->path, HTTPMessage::HTTP_1_1);
        this->doGet();
    } else {
        cerr << "method invalid" << d_ptr->method << endl;
    }
}

void ClientWorker::doGet() {
    d_ptr->session->sendRequest(*d_ptr->request);
    std::istream& is = d_ptr->session->receiveResponse(*d_ptr->response);
    std::string out_str = std::string(std::istreambuf_iterator<char>(is),{});
    d_ptr->res_data = out_str;
    std::cout << d_ptr->response->getStatus() << " " << d_ptr->response->getReason() << " " << out_str << std::endl;
}

void ClientWorker::doPost() {
    d_ptr->reqBody = "[{" + d_ptr->param + "}]";
    d_ptr->request->setContentLength(d_ptr->reqBody.length());
    std::ostream& os = d_ptr->session->sendRequest(*d_ptr->request);
    os << d_ptr->reqBody; // sends the body
    d_ptr->request->write(std::cout);

    std::istream& is = d_ptr->session->receiveResponse(*d_ptr->response);
    std::string out_str = std::string(std::istreambuf_iterator<char>(is),{});
    d_ptr->res_data = out_str;
    std::cout << d_ptr->response->getStatus() << " " << d_ptr->response->getReason() << " " << out_str << std::endl;

}

std::string ClientWorker::responseData() {
    std::cout << "response data: " << d_ptr->res_data << std::endl;
    return d_ptr->res_data;
}