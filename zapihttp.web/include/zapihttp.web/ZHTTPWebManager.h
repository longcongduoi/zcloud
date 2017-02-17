/*
 * File:   ZHTTPWebManager.h
 * Author: hoanh
 *
 * Created on February 20, 2016
 */

#ifndef __ZHTTPWEBMANAGER_H__
#define __ZHTTPWEBMANAGER_H__

#include <memory>
#include <vector>
#include <zdevice/ZDeviceInfo.h>

class ZHTTPWidget;
class ZDeviceInfo;

//! Quản lý User Preferences cho webapp
class ZHTTPWebManager {
public:
    ZHTTPWebManager()
    {}
public:
    //! Thêm một widget vào dashboard
    bool addWidget(int32_t userId, ZDeviceInfo::Ptr device);
    bool addWidget(int32_t userId, std::shared_ptr<ZHTTPWidget> widget);

    std::shared_ptr<ZHTTPWidget> getWidget(int32_t userId, int32_t widgetId);

    //! Xoá widget khỏi dashboard
    bool removeWidget(int32_t userId, int32_t widgetId);

    //! Cập nhật thông tin của widget trên dashboard.
    //! Cần: tham số userId, widgetId
    //! Cập nhật: deviceId, variableId
    bool updateWidget(int32_t userId, std::shared_ptr<ZHTTPWidget> widget);

    //! Lấy danh sách tất cả các widgets có trên dashboard
    std::vector<std::shared_ptr<ZHTTPWidget>> getAllWidgets(int32_t userId);

    //! Xoá tất cả các widgets trên dashboard
    bool removeAllWidgets(int32_t userId);
private:
    std::shared_ptr<ZHTTPWidget> getWidget(const std::string &widgetKey);
};

#endif // __ZHTTPWEBMANAGER_H__
