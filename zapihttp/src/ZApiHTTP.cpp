/*
 * File:   ZApiHTTP.cpp
 * Author: huuhoa
 *
 * Created on October 24, 2015, 3:18 PM
 */

#include "zapihttp/ZApiHTTP.h"
#include <Poco/Util/Application.h>
#include <Poco/Util/LayeredConfiguration.h>
#include <Poco/Timestamp.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/ThreadPool.h>
#include <Poco/Net/HTTPServer.h>
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"

#include <zapihttp.web/ZHTTPWebManager.h>
#include "ZApiHTTPRequestHandlerFactory.h"

using Poco::Net::ServerSocket;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;

using Poco::ThreadPool;
using Poco::DateTimeFormat;
using Poco::Timestamp;
using Poco::DateTimeFormatter;

using Poco::Util::Application;
using Poco::Util::LayeredConfiguration;

class ZApiHTTP::Impl {
public:
    Poco::SharedPtr<HTTPServer> httpServer;
    Poco::SharedPtr<ServerSocket> serverSocket;
    Poco::SharedPtr<ZHTTPWebManager> webManager;
};

ZApiHTTP::ZApiHTTP() :
    d_ptr(new Impl)
{
}

ZApiHTTP::~ZApiHTTP() {
}

bool ZApiHTTP::initialize() {
    LayeredConfiguration& config = Application::instance().config();
    // get parameters from configuration file
    unsigned short port = (unsigned short) config.getInt("api.http.port", 9980);
    std::string format(config.getString("api.http.format", DateTimeFormat::SORTABLE_FORMAT));
    int maxQueued  = config.getInt("api.http.maxQueued", 100);
    int maxThreads = config.getInt("api.http.maxThreads", 16);
    std::string httpApiPrefix = config.getString("api.http.prefix", "/api");
    ThreadPool::defaultPool().addCapacity(maxThreads);

    HTTPServerParams* pParams = new HTTPServerParams;
    pParams->setMaxQueued(maxQueued);
    pParams->setMaxThreads(maxThreads);

    d_ptr->webManager = new ZHTTPWebManager();

    // set-up a server socket
    d_ptr->serverSocket = new ServerSocket(port);
    // set-up a HTTPServer instance
    d_ptr->httpServer = new HTTPServer(
                new ZApiHTTPRequestHandlerFactory(format, httpApiPrefix, this),
                *(d_ptr->serverSocket.get()),
                pParams);
    return true;
}

bool ZApiHTTP::start() {
    // start the HTTPServer
    d_ptr->httpServer->start();
    return true;
}

bool ZApiHTTP::stop() {
    // Stop the HTTPServer
    d_ptr->httpServer->stop();
    return true;
}

bool ZApiHTTP::cleanup() {
    return true;
}

ZHTTPWebManager* ZApiHTTP::webManager() const {
    return d_ptr->webManager;
}
