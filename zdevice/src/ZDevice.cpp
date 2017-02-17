/* 
 * File:   ZDevice.cpp
 * Author: huuhoa
 * 
 * Created on November 30, 2015, 2:22 PM
 */

#include "zdevice/ZDevice.h"

ZDevice::ZDevice() {
}

ZDevice::ZDevice(const ZDevice& orig) :
    info_(orig.info_)
{
}

ZDevice::~ZDevice() {
}

