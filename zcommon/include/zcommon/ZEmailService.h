/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ZEmailService.h
 * Author: tamvh
 *
 * Created on December 20, 2016, 6:57 PM
 */

#ifndef ZEMAILSERVICE_H
#define ZEMAILSERVICE_H
#include <memory>
#include <iostream>
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/MailRecipient.h>
#include <Poco/Net/SMTPClientSession.h>
#include <Poco/Net/NetException.h>
using namespace std;
using namespace Poco::Net;
using namespace Poco;
class ZEmailService {
public:
    ZEmailService(
            std::string host,
            Poco::UInt16 port,
            std::string user,
            std::string pass,
            std::string emailfrom,
            std::string emailto,
            std::string subject);
    virtual ~ZEmailService();
public:    
    int32_t send();
private:
    std::string _host;
    Poco::UInt16 _port;    
    std::string _user;    
    std::string _pass;    
    std::string _emailfrom;    
    std::string _emailto;    
    std::string _subject; 
    std::string _content;
    MailMessage *_msg;
    SMTPClientSession *_session;
};

#endif /* ZEMAILSERVICE_H */

