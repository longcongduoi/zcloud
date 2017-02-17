/* 
 * File:   ZadminManager.h
 * Author: tamvh
 *
 * Created on November 27, 2015, 12:04 PM
 */

#ifndef ZADMINMANAGER_H
#define ZADMINMANAGER_H
#include <zcommon/ZServiceInterface.h>
#include <zadmin/ZAdminInfo.h>

class ZAdminManager : public ZServiceInterface {
public:
    ZAdminManager();
    virtual ~ZAdminManager();

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
    virtual int32_t create(const std::string& userName, const std::string& password, const std::string& displayName, const std::string& avatar);

    /// Lấy thông tin về một tài khoản có sẵn
    virtual ZAdminInfo::Ptr get(int32_t userId);
    virtual ZAdminInfo::Ptr get(const std::string& userName);
//    virtual ZAdminInfo::Ptr getWithUName(const std::string& userName);

    /// Lấy thông tin một số tài khoản theo danh sách khoá
    virtual ZAdminInfo::Map multiGet(const ZAdminInfo::KeyList& keyList);

    /// Lấy thông tin một số tài khoản theo danh sách tuần tự
    virtual ZAdminInfo::Map list(int32_t start, int32_t count);

    /// Xoá tài khoản khỏi DB
    virtual ErrorCode remove(int32_t userId);

    /// Cập nhật thông tin về tài khoản
    /// Trả về mã lỗi nếu không thành công
    virtual ErrorCode update(ZAdminInfo::Ptr adminInfo);
private:
    void saveToDB(ZAdminInfo& adminInfo);
};

#endif /* ZADMINMANAGER_H */

