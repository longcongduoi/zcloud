#ifndef __ZDEVICE_ZDBKEY_H__
#define __ZDEVICE_ZDBKEY_H__

#include <string>

class ZDBKey {
public:
    //KEY generator
    static std::string generatorUsers();
    static std::string generatorDevices();
    static std::string generatorAdmin();

    static std::string DevicesSet();
    static std::string DevicesApiKey();
    static std::string DevicesFactory();
    static std::string DeviceEntry(int32_t deviceId);

    static std::string VarInfoEntry(int32_t varId);
    static std::string VarLastestEntry();
    static std::string VarLastestBatteryEntry();
    static std::string VarLastestEntry(int32_t deviceId);
    static std::string VarLastestBatteryEntry(int32_t deviceId);
    static std::string VarValueList(int32_t deviceId);
    static std::string VarValueArr(int32_t deviceId);
    static std::string VarValueEntry(int32_t deviceId, uint64_t timestamp);

    static std::string UserSet();
    static std::string UserApiKey();
    static std::string UserEmail();
    static std::string UserEntry(int32_t userId);
    static std::string UserEntry(const std::string& apikey);
    static std::string UserSession(int32_t userId);

    static std::string AdminSet();
    static std::string AdminApiKey();
    static std::string AdminEmail();
    static std::string AdminEntry(int32_t adminId);

    static std::string apiKey();
    static std::string addressKey();
    static std::string relationDeviceVars(int32_t deviceId);
    static std::string relationDeviceVars(const std::string& address, int32_t deviceType);
    static std::string relationUserDevices(int32_t userId);
};

#endif // __ZDEVICE_ZDBKEY_H__
