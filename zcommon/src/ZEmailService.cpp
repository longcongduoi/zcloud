/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ZEmailService.cpp
 * Author: tamvh
 * 
 * Created on December 20, 2016, 6:57 PM
 */

#include "zcommon/ZEmailService.h"

ZEmailService::ZEmailService(
            std::string host,
            Poco::UInt16 port,
            std::string user,
            std::string pass,
            std::string emailfrom,
            std::string emailto,
            std::string subject):
        _host(host),
        _port(port),
        _user(user),
        _pass(pass),
        _emailfrom(emailfrom),
        _emailto(emailto),
        _subject(subject)
{        
    _subject = MailMessage::encodeWord(_subject, "UTF-8");
    _msg->setSender(_emailfrom);
    _msg->addRecipient(MailRecipient(MailRecipient::PRIMARY_RECIPIENT, _emailto));
    _msg->setSubject(_subject);
    _msg->setContentType("text/plain; charset=UTF-8");
    _msg->setContent(_content, MailMessage::ENCODING_8BIT);
    _session = new SMTPClientSession(_host, _port);
    
}

ZEmailService::~ZEmailService() {
    
}

int ZEmailService::send() {
    try {
        _session->open();
        _session->login(SMTPClientSession::AUTH_LOGIN, _user, _pass);
        _session->sendMessage(*_msg);
        cout << "Message successfully sent" << endl;
        _session->close();
    } catch (NetException ex) {
        cerr << "Send email exception: " << ex.displayText() << endl;
        _session->close();
        return 0;
    }
    return 0;
}



