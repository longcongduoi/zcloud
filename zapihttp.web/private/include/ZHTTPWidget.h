/*
 * File:   ZHTTPWidget.h
 * Author: hoanh
 *
 * Created on February 20, 2016
 */

#ifndef __ZHTTPWIDGET_H__
#define __ZHTTPWIDGET_H__

#include <memory>
#include <vector>
#include <string>

//! Widget dùng để hiển thị trên dashboard cho webapp
class ZHTTPWidget {
public:
    ZHTTPWidget()
    {}
public:
    int32_t widgetId() const;
    int32_t widgetType() const;
    std::string widgetName() const;
    int32_t widgetSize() const;
    int32_t deviceId() const;


    void setWidgetId(const int32_t& value);
    void setWidgetType(const int32_t& value);
    void setWidgetName(const std::string& value);
    void setWidgetSize(const int32_t& value);
    void setDeviceId(const int32_t& value);
    
private:
    int32_t widgetId_;
    int32_t widgetType_;
    std::string widgetName_;
    int32_t widgetSize_;
    int32_t deviceId_;
};

#endif // __ZHTTPWIDGET_H__
