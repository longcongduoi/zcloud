/*
 * File:   ZIdGenerator.h
 * Author: huuhoa
 *
 * Created on October 27, 2015, 3:59 PM
 */

#ifndef ZIDGENERATOR_H
#define	ZIDGENERATOR_H

#include <string>
#include <map>

class ZIdGenerator {
public:
    ZIdGenerator();
public:
    int32_t getNext(const std::string& collectionName);
    std::string createUuid() const;
};

#endif	/* ZIDGENERATOR_H */

