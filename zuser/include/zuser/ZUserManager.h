/* 
 * File:   ZUserManager.h
 * Author: huuhoa
 *
 * Created on October 23, 2015, 2:05 PM
 */

#ifndef ZUSERMANAGER_H
#define ZUSERMANAGER_H

#include <zcommon/ZServiceInterface.h>
#include <zuser/ZUserInfo.h>

class ZUserManager : public ZServiceInterface {
public:
    ZUserManager();
    virtual ~ZUserManager();

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
    virtual ZUserInfo::Ptr get(int32_t userId);
    virtual ZUserInfo::Ptr getWithUserKey(const std::string& userkey); //userkey = "users:21"
    virtual ZUserInfo::Ptr get(const std::string& userName); //userName = email

    /// Lấy thông tin một số tài khoản theo danh sách khoá
    virtual ZUserInfo::Map multiGet(const ZUserInfo::KeyList& keyList);

    /// Lấy thông tin một số tài khoản theo danh sách tuần tự
    virtual ZUserInfo::Map list(int32_t start, int32_t count);

    /// Xoá tài khoản khỏi DB
    virtual ErrorCode remove(int32_t userId);

    /// Cập nhật thông tin về tài khoản
    /// Trả về mã lỗi nếu không thành công
    virtual ErrorCode update(ZUserInfo::Ptr userInfo);
private:
    void saveToDB(ZUserInfo& userInfo);
};

#endif /* ZUSERMANAGER_H */

