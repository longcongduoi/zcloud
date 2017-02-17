/*
 * File:   ErrorCode.h
 * Author: huuhoa
 *
 * Created on November 3, 2015, 4:58 PM
 */

#ifndef ZCOMMON_ERRORCODE_H
#define ZCOMMON_ERRORCODE_H

#include <cstdint>

enum class ZSensorType : int32_t {
    POWER_SWITCH = 1,
    LAMP_ONOFF = 2,
    TEMP_SENSOR = 3,
    HUMIDITY_SENSOR = 4,
    LIGHT_SENSOR = 5
};


#endif /* ZCOMMON_ERRORCODE_H */

