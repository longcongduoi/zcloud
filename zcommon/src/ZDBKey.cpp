#include <sstream>
#include "zcommon/ZDBKey.h"

std::string ZDBKey::generatorUsers() {
    return "users";
}

std::string ZDBKey::generatorDevices() {
    return "devices";
}

std::string ZDBKey::generatorAdmin() {
    return "admin";
}

std::string ZDBKey::DevicesSet() {
    return "devices:set";
}

std::string ZDBKey::DevicesApiKey() {
    return "devices:lookup:apikey";
}

std::string ZDBKey::DevicesFactory() {
    return "factorydevices:set";
}

std::string ZDBKey::DeviceEntry(int32_t deviceId) {
    std::stringstream stream;
    stream << "devices:" << deviceId;
    return stream.str();
}

std::string ZDBKey::VarInfoEntry(int32_t varId) {
    std::stringstream stream;
    stream << "vars:" << varId << ":info";
    return stream.str();
}

std::string ZDBKey::VarLastestEntry() {
    return "varlastes";
}

std::string ZDBKey::VarLastestBatteryEntry() {
    return "battery:varlastes";
}

std::string ZDBKey::VarLastestEntry(int32_t deviceId) {
    std::stringstream stream;
    stream << "vars:" << deviceId << ":lastest";
    return stream.str();
}

std::string ZDBKey::VarLastestBatteryEntry(int32_t deviceId) {
    std::stringstream stream;
    stream << "vars:" << deviceId << ":battery:lastest";
    return stream.str();
}

std::string ZDBKey::VarValueList(int32_t deviceId) {
    std::stringstream stream;
    stream << "vars:" << deviceId << ":list";
    return stream.str();
}

std::string ZDBKey::VarValueArr(int32_t deviceId) {
    std::stringstream stream;
    stream << "vars:" << deviceId << ":array";
    return stream.str();
}

std::string ZDBKey::VarValueEntry(int32_t deviceId, uint64_t timestamp) {
    std::stringstream stream;
    stream << "vars:" << deviceId << ":" << timestamp;
    return stream.str();
}

std::string ZDBKey::UserApiKey() {
    return "users:lookup:apikey";
}

std::string ZDBKey::UserEmail() {
    return "users:lookup:email";
}

std::string ZDBKey::UserSet() {
    return "users:set";
}

std::string ZDBKey::UserEntry(int32_t userId) {
    std::stringstream stream;
    stream << "users:" << userId;
    return stream.str();
}

std::string ZDBKey::UserEntry(const std::string& apikey) {
    std::stringstream stream;
    stream << "users.apikey:" << apikey;
    return stream.str();
}

std::string ZDBKey::AdminSet() {
    return "admin:set";
}

std::string ZDBKey::AdminApiKey() {
    return "admin:lookup:apikey";
}

std::string ZDBKey::AdminEmail() {
    return "admin:lookup:email";
}

std::string ZDBKey::AdminEntry(int32_t adminId) {
    std::stringstream stream;
    stream << "admin:" << adminId;
    return stream.str();
}

std::string ZDBKey::apiKey() {
    return "apikey";
}

std::string ZDBKey::addressKey() {
    return "addresskey";
}

std::string ZDBKey::relationDeviceVars(int32_t deviceId) {
    std::stringstream stream;
    stream << "devices:" << deviceId << ":vars";
    return stream.str();
}

std::string ZDBKey::relationDeviceVars(const std::string& address, int32_t deviceType) {
    std::stringstream stream;
    stream << "address:" << address << ":type:"<< deviceType << ":vars";
    return stream.str();
}

std::string ZDBKey::relationUserDevices(int32_t userId) {
    std::stringstream stream;
    stream << "users:" << userId << ":devices";
    return stream.str();
}

std::string ZDBKey::UserSession(int32_t userId) {
    std::stringstream stream;
    stream << "users:" << userId << ":sessions";
    return stream.str();
}
