/*
 * File:   ZHTTPWebManager.cpp
 * Author: hoanh
 *
 * Created on February 20, 2016
 */

#include <string>
#include <sstream>

#include <Poco/NumberParser.h>

#include <zcommon/ZServiceLocator.h>
#include <zcommon/ZDBKey.h>

#include <zdb/ZDBProxy.h>
#include <zdb/ZIdGenerator.h>

#include "ZHTTPWidget.h"
#include <zapihttp.web/ZHTTPWebManager.h>
#include <stdint.h>

namespace {
std::string widgetKey(int32_t widgetId) {
    std::stringstream stream;
    stream << "widgets:" << widgetId << ":data";
    return stream.str();
}

std::string widgetListForUser(int32_t userId) {
    std::stringstream stream;
    stream << "users:" << userId << ":widgets";
    return stream.str();
}

int32_t getInt32FromList(const std::vector<std::string> &list, int position, int32_t defaultValue) {
    if (list.size() <= position) {
        return defaultValue;
    }

    int32_t value;
    if (Poco::NumberParser::tryParse(list[position], value)) {
        return value;
    } else {
        return defaultValue;
    }
}

std::string getStringFromList(const std::vector<std::string> &list, int position, const std::string& defaultValue) {
    if (list.size() <= position) {
        return defaultValue;
    }

    return list[position];
}

} // end namespace anonymous

bool ZHTTPWebManager::addWidget(int32_t userId, ZDeviceInfo::Ptr device) {
    ZDBProxy* db = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    int32_t widgetId = device->deviceId();
    std::shared_ptr<ZHTTPWidget> widget(new ZHTTPWidget);
    widget->setWidgetId(widgetId);
    widget->setWidgetType(device->deviceType());
    widget->setWidgetName(device->deviceName());
    widget->setDeviceId(device->deviceId());
    int32_t widgetSize = 1;
    widget->setWidgetSize(widgetSize);
    
    std::string key = widgetListForUser(userId);
    std::string wk = widgetKey(widget->widgetId());

    // Add widget detail
    db->HMSET(wk, {
                  { "id", std::to_string(widget->widgetId()) },
                  { "type", std::to_string(widget->widgetType()) },
                  { "name", widget->widgetName() },
                  { "device_id", std::to_string(widget->deviceId()) },
                  { "size", std::to_string(widget->widgetSize()) },
              });

    // Record widget relationship
    db->SADD(key, { wk });
    return true;
}


bool ZHTTPWebManager::addWidget(int32_t userId, std::shared_ptr<ZHTTPWidget> widget) {
    ZDBProxy* db = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    ZIdGenerator* generator = ZServiceLocator::instance()->get<ZIdGenerator>(ZServiceLocator::ServiceId::IDGenerator);
//    int32_t widgetId = generator->getNext("widgets");
    int32_t widgetId = widget->deviceId();
    widget->setWidgetId(widgetId);

    std::string key = widgetListForUser(userId);
    std::string wk = widgetKey(widget->widgetId());

    // Add widget detail
    db->HMSET(wk, {
                  { "id", std::to_string(widget->widgetId()) },
                  { "type", std::to_string(widget->widgetType()) },
                  { "name", widget->widgetName() },
                  { "device_id", std::to_string(widget->deviceId()) },
                  { "size", std::to_string(widget->widgetSize()) },
              });

    // Record widget relationship
    db->SADD(key, { wk });
    return true;
}

std::shared_ptr<ZHTTPWidget> ZHTTPWebManager::getWidget(int32_t userId, int32_t widgetId) {
    ZDBProxy* db = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    std::string key = widgetListForUser(userId);
    std::string wk = widgetKey(widgetId);
    if (!db->SISMEMBER(key, wk) || !db->EXISTS(wk)) {
        return std::shared_ptr<ZHTTPWidget>();
    }

    return getWidget(wk);
}

bool ZHTTPWebManager::removeWidget(int32_t userId, int32_t widgetId) {
    ZDBProxy* db = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    std::string key = widgetListForUser(userId);
    std::string wk = widgetKey(widgetId);
    db->DEL(wk);
    db->SREM(key, { wk });

    return true;
}

bool ZHTTPWebManager::updateWidget(int32_t userId, std::shared_ptr<ZHTTPWidget> widget) {
    ZDBProxy* db = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    std::string key = widgetListForUser(userId);
    std::string wk = widgetKey(widget->widgetId());
    if (!db->SISMEMBER(key, wk)) {
        return false;
    }

    if (!db->EXISTS(wk)) {
        return false;
    }

    db->HMSET(wk, {
                  { "name", widget->widgetName() },
                  { "device_id", std::to_string(widget->deviceId()) },
                  { "size", std::to_string(widget->widgetSize()) }
              });
    return true;
}

std::vector<std::shared_ptr<ZHTTPWidget>> ZHTTPWebManager::getAllWidgets(int32_t userId) {
    ZDBProxy* db = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    std::vector<std::shared_ptr<ZHTTPWidget>> result;

    // get list of all widgets associated with userId
    std::string key = widgetListForUser(userId);
    ZDBProxy::StringList keyWidgets = db->SMEMBERS(key);
    if (keyWidgets.empty()) {
        return result;
    }

    // get all informations
    for (std::string wk : keyWidgets) {
        std::shared_ptr<ZHTTPWidget> widget = getWidget(wk);
        if (!widget) {
            continue;
        }

        result.push_back(widget);
    }

    return result;
}

bool ZHTTPWebManager::removeAllWidgets(int32_t userId) {
    ZDBProxy* db = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    // get list of all widgets associated with userId
    std::string key = widgetListForUser(userId);
    ZDBProxy::StringList keyWidgets = db->SMEMBERS(key);
    if (keyWidgets.empty()) {
        return false;
    }

    // remove all widgets
    db->DEL(keyWidgets);

    // remove metadata
    db->DEL(key);

    return true;
}

std::shared_ptr<ZHTTPWidget> ZHTTPWebManager::getWidget(const std::string &widgetKey) {
    ZDBProxy* db = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    ZDBProxy::StringList values = db->HMGET(widgetKey, { "id", "type", "device_id", "name", "size" });
    std::shared_ptr<ZHTTPWidget> widget(new ZHTTPWidget);
    widget->setWidgetId(getInt32FromList(values, 0, -1));
    widget->setWidgetType(getInt32FromList(values, 1, -1));
    widget->setDeviceId(getInt32FromList(values, 2, -1));
    widget->setWidgetName(getStringFromList(values, 3, ""));
    widget->setWidgetSize(getInt32FromList(values, 4, -1));

    return widget;
}
