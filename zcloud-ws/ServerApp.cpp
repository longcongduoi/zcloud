/* 
 * File:   ServerApp.cpp
 * Author: huuhoa
 * 
 * Created on October 24, 2015, 3:00 PM
 */

#include "ServerApp.h"
#include <Poco/Util/Application.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/IntValidator.h>
#include <Poco/Util/HelpFormatter.h>
#include <iostream>

#include <zapihttp/ZApiHTTP.h>
#include <zapimqtt/ZApiMQTT.h>
#include <zapiws/ZApiWS.h>

#include <zuser/ZUserManager.h>
#include <zdevice/ZDeviceManager.h>
#include <zadmin/ZAdminManager.h>

#include <zcommon/ZServiceLocator.h>
#include <zdb/ZDBProxy.h>
#include <zcommon/ZMemProxy.h>
#include <zcommon/ZMemProxyLocal.h>
#include <zdb/ZIdGenerator.h>
#include <zsession/ZSessionService.h>
#include <zworker/ZWorker.h>

class ServerApp::Impl {
public:

    Impl() :
    userManager(new ZUserManager()),
    deviceManager(new ZDeviceManager()),
    worker(new ZWorker()),
    dbProxy(new ZDBProxy),
    memcachedProxy(new ZMemProxyLocal()),
    idGenerator(new ZIdGenerator()),
    sessionService(new ZSessionService()),
    adminManager(new ZAdminManager()) {
        ZServiceLocator::instance()->registerService(ZServiceLocator::ServiceId::UserManager, userManager.get());
        ZServiceLocator::instance()->registerService(ZServiceLocator::ServiceId::DeviceManager, deviceManager.get());
        ZServiceLocator::instance()->registerService(ZServiceLocator::ServiceId::DBProxy, dbProxy.get());
        ZServiceLocator::instance()->registerService(ZServiceLocator::ServiceId::MemCacheProxy, memcachedProxy.get());
        ZServiceLocator::instance()->registerService(ZServiceLocator::ServiceId::IDGenerator, idGenerator.get());
        ZServiceLocator::instance()->registerService(ZServiceLocator::ServiceId::SessionService, sessionService.get());
        ZServiceLocator::instance()->registerService(ZServiceLocator::ServiceId::Worker, worker.get());
        ZServiceLocator::instance()->registerService(ZServiceLocator::ServiceId::AdminManager, adminManager.get());

//        services.push_back(new ZApiHTTP());
        services.push_back(new ZApiMQTT());
        services.push_back(new ZApiWS());

        services.push_back(userManager);
        services.push_back(deviceManager);
        services.push_back(worker);
        services.push_back(adminManager);
    }
public:
    Poco::SharedPtr<ZUserManager> userManager;
    Poco::SharedPtr<ZDeviceManager> deviceManager;
    Poco::SharedPtr<ZWorker> worker;

    Poco::SharedPtr<ZDBProxy> dbProxy;
    Poco::SharedPtr<ZMemProxy> memcachedProxy;
    Poco::SharedPtr<ZIdGenerator> idGenerator;
    Poco::SharedPtr<ZSessionService> sessionService;
    Poco::SharedPtr<ZAdminManager> adminManager;

    typedef Poco::SharedPtr<ZServiceInterface> ZServiceInterfacePtr;
    typedef std::vector<ZServiceInterfacePtr> ZServiceList;
    typedef std::vector<ZServiceInterfacePtr>::iterator ServiceIterator;
    typedef std::vector<ZServiceInterfacePtr>::reverse_iterator ServiceReverseIterator;

    std::vector<ZServiceInterfacePtr> services;    
};

ServerApp::ServerApp()
: _helpRequested(false) {
}

ServerApp::~ServerApp() {

}

void ServerApp::initialize(Application& self) {
    loadConfiguration(); // load default configuration files, if present
    ServerApplication::initialize(self);
}

void ServerApp::uninitialize() {
    ServerApplication::uninitialize();
}

void ServerApp::defineOptions(Poco::Util::OptionSet& options) {
    ServerApplication::defineOptions(options);

    options.addOption(
            Poco::Util::Option("help", "h", "display help information on command line arguments")
            .required(false)
            .repeatable(false));

    options.addOption(
        Poco::Util::Option("port", "p", "Set http api listening port")
            .required(false)
            .argument("value")
            .validator(new Poco::Util::IntValidator(1000, 65535))
            .binding("api.http.port"));
}

void ServerApp::handleOption(const std::string& name, const std::string& value) {
    ServerApplication::handleOption(name, value);

    if (name == "help") {
        _helpRequested = true;
    }
}

void ServerApp::displayHelp() {
    Poco::Util::HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader("A server application that serves the current date and time.");
    helpFormatter.format(std::cout);
}

int ServerApp::main(const std::vector<std::string>& args) {
    (void) args;
    if (_helpRequested) {
        displayHelp();
    } else {
        d_ptr = new Impl();

        std::string dbHost = config().getString("proxy.db.host", "localhost");
        int dbPort = config().getInt("proxy.db.port", 6379);
        d_ptr->dbProxy->connect(dbHost, dbPort);
        if (!d_ptr->dbProxy->isConnected()) {
            logger().error("Cannot connect to DB Proxy. Review configuration option for proxy.db.host and proxy.db.port. Current values are proxy.db.host=[%s] and proxy.db.port=[%d]",
                    dbHost, dbPort);
            return Application::EXIT_CONFIG;
        }
        for (Impl::ServiceIterator iter = d_ptr->services.begin();
                iter != d_ptr->services.end();
                iter++) {
            (*iter)->initialize();
        }

        for (Impl::ServiceIterator iter = d_ptr->services.begin();
                iter != d_ptr->services.end();
                iter++) {
            (*iter)->start();
        }

        // get parameters from configuration file
        //        _messageBus = new iot::messagebus::MessageBus();
        //        _messageBus->initialize(config());
        //
        //        _service = iot::service::ServiceFactory::createService();
        //        _service->setLogger(logger());
        //        _service->initialize(_messageBus);
        //        _service->start();
        // wait for CTRL-C or kill
        logger().information("Service is started");
        waitForTerminationRequest();

        // Stop the TCPServer
        //        _service->stop();
        for (Impl::ServiceReverseIterator riter = d_ptr->services.rbegin();
                riter != d_ptr->services.rend();
                riter++) {
            (*riter)->stop();
        }

        for (Impl::ServiceReverseIterator riter = d_ptr->services.rbegin();
                riter != d_ptr->services.rend();
                riter++) {
            (*riter)->cleanup();
        }
        logger().information("Service is stoped");
    }
    return Application::EXIT_OK;
}


