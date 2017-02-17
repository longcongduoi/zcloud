/*
 * File:   ZApiHTTP.h
 * Author: huuhoa
 *
 * Created on October 24, 2015, 3:18 PM
 */

#ifndef ZAPIHTTP_H
#define	ZAPIHTTP_H

#include <memory>
#include <zcommon/ZServiceInterface.h>

class ZHTTPWebManager;

class ZApiHTTP : public ZServiceInterface {
public:
    ZApiHTTP();
    virtual ~ZApiHTTP();

public:
    virtual bool initialize();
    virtual bool start();
    virtual bool stop();
    virtual bool cleanup();

public:
    ZHTTPWebManager* webManager() const;
private:
    class Impl;
    std::shared_ptr<Impl> d_ptr;
};

#endif	/* ZAPIHTTP_H */

