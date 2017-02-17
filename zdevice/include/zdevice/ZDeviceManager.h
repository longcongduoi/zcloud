/*
 * File:   ZDeviceManager.h
 * Author: huuhoa
 *
 * Created on October 24, 2015, 3:20 PM
 */

#ifndef ZDEVICEMANAGER_H
#define	ZDEVICEMANAGER_H

#include <zcommon/ZServiceInterface.h>
#include <zdevice/ZDeviceInfo.h>
#include <zdevice/ZVarInfo.h>

#include "ZVarInfo.h"
#include "ZVarValue.h"
#include "ZVarLastestValue.h"


class ZDeviceManager : public ZServiceInterface {
public:
    ZDeviceManager();
    virtual ~ZDeviceManager();

public:
    virtual bool initialize();
    virtual bool start();
    virtual bool stop();
    virtual bool cleanup();
public:
    enum class ErrorCode : int32_t {
        OK = 0,
        NotFound,
        InvalidInput
    };

public:
    /// Tạo mới tài khoản và lưu vào DB
    /// @return userId nếu tạo tài khoản thành công.
    /// trường hợp xảy ra lỗi, trả về negative của mã lỗi
    virtual int32_t createDeviceInfo(
            const std::string& deviceName,
            const std::string& deviceAddress,
            int32_t deviceType, 
            const std::string& deviceGroup);
    virtual int32_t createFactoryDeviceInfo(const std::string& deviceName, int32_t deviceType, const std::string& deviceGroup);
    virtual int32_t createVarInfo(ZVarInfo varInfo, const std::string& key);
    virtual int32_t createVarValue(ZVarValue varValue, int32_t deviceId);
    virtual int32_t createBatteryValue(int32_t deviceId, int32_t battery);
    virtual int32_t createVarLastestValue(ZVarLastestValue varLastestValue);
    virtual ZVarLastestValue::Ptr getVarLastestValue(int32_t varId);
    virtual ZVarInfo::Ptr getVarInfo(const std::string& key);
    virtual ZVarValue::Ptr getVarValue(const std::string& key);
    virtual ZVarLastestValue::Map mGetVarLastestInfo(const ZVarLastestValue::KeyList& keyList);
    virtual ZVarValue::MapString mGetVarValue(const ZVarValue::KeyListString& keyListString);
    virtual ZVarValue::ListString mGetVarValue(const std::string& key, int32_t start, int32_t end);

    /// Lấy thông tin về một tài khoản có sẵn
    virtual ZDeviceInfo::Ptr getDeviceInfo(int32_t deviceId);
    virtual ZDeviceInfo::Ptr getDeviceInfo(const std::string& address, int32_t sensor);
    virtual ZDeviceInfo::Ptr getDeviceInfo(const std::string& deviceKey);


    /// Lấy thông tin một số tài khoản theo danh sách khoá
    virtual ZDeviceInfo::Map mGetDeviceInfo(const ZDeviceInfo::KeyList& keyList);
    virtual ZDeviceInfo::Map mGetDeviceInfo(const ZDeviceInfo::KeyListString& keyList);


    /// Lấy thông tin một số tài khoản theo danh sách tuần tự
    virtual ZDeviceInfo::Map list(int32_t start, int32_t count);

    /// Xoá device khỏi DB
    virtual ErrorCode removeDeviceInfo(int32_t deviceId);
    virtual ErrorCode removeVarInfo(int32_t varId);
    virtual ErrorCode removeVarLastestValue(int32_t varId);
    virtual ErrorCode removeVarValue(int32_t deviceId);

    /// Cập nhật thông tin về tài khoản
    /// Trả về mã lỗi nếu không thành công
    virtual ErrorCode updateDeviceInfo(ZDeviceInfo::Ptr deviceInfo);
private:
    void saveDeviceInfoToDB(ZDeviceInfo& deviceInfo);
    void saveVarInfoToDB(ZVarInfo& varInfo, const std::string& key);
    void saveVarValueToDB(ZVarValue& varValue, int32_t deviceId);
    void saveVarLastetValueToDB(ZVarLastestValue& varLastestValue);
};

#endif	/* ZDEVICEMANAGER_H */

