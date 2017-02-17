/*
 * File:   ZHTTPWidget.cpp
 * Author: hoanh
 *
 * Created on February 20, 2016
 */

#include "ZHTTPWidget.h"

int32_t ZHTTPWidget::widgetId() const {
    return widgetId_;
}

int32_t ZHTTPWidget::widgetType() const {
    return widgetType_;
}

std::string ZHTTPWidget::widgetName() const
{
    return widgetName_;
}

int32_t ZHTTPWidget::widgetSize() const {
    return widgetSize_;
}

int32_t ZHTTPWidget::deviceId() const {
    return deviceId_;
}

void ZHTTPWidget::setWidgetId(const int32_t& value) {
    widgetId_ = value;
}

void ZHTTPWidget::setWidgetType(const int32_t& value) {
    widgetType_ = value;
}

void ZHTTPWidget::setWidgetName(const std::string& value)
{
    widgetName_ = value;
}

void ZHTTPWidget::setWidgetSize(const int32_t& value)
{
    widgetSize_ = value;
}

void ZHTTPWidget::setDeviceId(const int32_t& value) {
    deviceId_ = value;
}
