
//
// Created by Nguyễn Hữu Hoà on 10/23/15.
//

#include <sstream>
#include <zuser/ZUserInfo.h>

ZUserInfo::ZUserInfo():
    userId_(0),
    createdAt_(0),
    updatedAt_(0)
{}

bool ZUserInfo::operator ==(const ZUserInfo& other) {
    return userId_ == other.userId_ &&
            userName_ == other.userName_ &&
            displayName_ == other.displayName_ &&
            avatar_ == other.avatar_ &&
            password_ == other.password_ &&
            createdAt_ == other.createdAt_ &&
            updatedAt_ == other.updatedAt_;
}

bool ZUserInfo::operator !=(const ZUserInfo& other) {
    return userId_ != other.userId_ ||
            userName_ != other.userName_ ||
            displayName_ != other.displayName_ ||
            avatar_ != other.avatar_ ||
            password_ != other.password_ ||
            createdAt_ != other.createdAt_ ||
            updatedAt_ != other.updatedAt_;
}

void ZUserInfo::setApiKey(const std::string& value) {
    this->apiKey_ = value;
}

std::string ZUserInfo::apiKey() const {
    return apiKey_;
}

void ZUserInfo::setUserId(const int32_t value) {
    this->userId_ = value;
}

int32_t ZUserInfo::userId() const {
    return userId_;
}

void ZUserInfo::setUserName(const std::string& value) {
    this->userName_ = value;
}

std::string ZUserInfo::userName() const {
    return userName_;
}

void ZUserInfo::setPassword(const std::string& value) {
    this->password_ = value;
}

std::string ZUserInfo::password() const {
    return password_;
}

void ZUserInfo::setDisplayName(const std::string& value) {
    this->displayName_ = value;
}

std::string ZUserInfo::displayName() const {
    return displayName_;
}

void ZUserInfo::setAvatar(const std::string& value) {
    this->avatar_ = value;
}

std::string ZUserInfo::avatar() const {
    return avatar_;
}

void ZUserInfo::setCreatedAt(const int64_t value) {
    this->createdAt_ = value;
}

int64_t ZUserInfo::createdAt() const {
    return createdAt_;
}

void ZUserInfo::setUpdatedAt(const int64_t value) {
    this->updatedAt_ = value;
}

int64_t ZUserInfo::updatedAt() const {
    return updatedAt_;
}

