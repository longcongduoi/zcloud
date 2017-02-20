/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: tamvh
 *
 * Created on January 10, 2016, 2:53 PM
 */

#include "ZApiHTTPClient.h"

using namespace std;

/*
 * 
 */
using namespace Poco;
using namespace Net;
bool doGet(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request, Poco::Net::HTTPResponse& response)
{
    session.sendRequest(request);
    std::istream& rs = session.receiveResponse(response);
    std::string out_str = std::string(std::istreambuf_iterator<char>(rs),{});
    std::cout << response.getStatus() << " " << response.getReason() << " " << out_str << std::endl;
    return true;
}

bool doPost(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request,Poco::Net::HTTPResponse& response)
{
    std::string reqBody("[{\"name\": \"iot\", \"timestamp\": 1487090604, \"value\": 122.5}]");
    request.setContentLength( reqBody.length() );
    std::ostream& myOStream = session.sendRequest(request);
    myOStream << reqBody;  // sends the body
    request.write(std::cout);
    
    std::istream& is = session.receiveResponse(response);
    std::string out_str = std::string(std::istreambuf_iterator<char>(is),{});
    std::cout << response.getStatus() << " " << response.getReason() << " " << out_str << std::endl;
    return true;
}

int main(int argc, char** argv) {
//    URI uri("http://iotlc.stats.vng.com.vn/api/datapoints?name=iot&from=1487090600&to=1487090610");
    URI uri("http://iotlc.stats.vng.com.vn/api/datapoints");
    std::string path(uri.getPathAndQuery());
    if (path.empty()) path = "/";
    
    std::string method = "POST";    
    HTTPClientSession session(uri.getHost(), uri.getPort()); 
    HTTPResponse response;
    bool result;
    if(method == "GET") {
        HTTPRequest request(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
        result = doGet(session, request, response);
    } else if(method == "POST") {
        HTTPRequest request(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
        result = doPost(session, request, response);
    } else {
        
    }    
    return 0;
}

