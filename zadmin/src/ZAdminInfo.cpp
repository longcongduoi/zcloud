/* 
 * File:   ZAdminInfo.cpp
 * Author: tamvh
 * 
 * Created on November 27, 2015, 12:03 PM
 */

#include "zadmin/ZAdminInfo.h"

ZAdminInfo::ZAdminInfo() :
    userId_(0),
    createdAt_(0),
    updatedAt_(0),
    isAdmin_(false)

{

}

bool ZAdminInfo::operator==(const ZAdminInfo& other) {
    return userId_ == other.userId_ &&
            userName_ == other.userName_ &&
            displayName_ == other.displayName_ &&
            avatar_ == other.avatar_ &&
            password_ == other.password_ &&
            createdAt_ == other.createdAt_ &&
            updatedAt_ == other.updatedAt_ &&
            isAdmin_ == other.isAdmin_;
}

bool ZAdminInfo::operator!=(const ZAdminInfo& other) {
    return userId_ != other.userId_ ||
            userName_ != other.userName_ ||
            displayName_ != other.displayName_ ||
            avatar_ != other.avatar_ ||
            password_ != other.password_ ||
            createdAt_ != other.createdAt_ ||
            updatedAt_ != other.updatedAt_ ||
            isAdmin_ != other.isAdmin_;
}

void ZAdminInfo::setApiKey(const std::string& value) {
    this->apiKey_ = value;
}

std::string ZAdminInfo::apiKey() const {
    return apiKey_;
}

void ZAdminInfo::setUserId(const int32_t value) {
    this->userId_ = value;
}

int32_t ZAdminInfo::userId() const {
    return userId_;
}

void ZAdminInfo::setUserName(const std::string& value) {
    this->userName_ = value;
}

std::string ZAdminInfo::userName() const {
    return userName_;
}

void ZAdminInfo::setPassword(const std::string& value) {
    this->password_ = value;
}

std::string ZAdminInfo::password() const {
    return password_;
}

void ZAdminInfo::setDisplayName(const std::string& value) {
    this->displayName_ = value;
}

std::string ZAdminInfo::displayName() const {
    return displayName_;
}

void ZAdminInfo::setAvatar(const std::string& value) {
    this->avatar_ = value;
}

std::string ZAdminInfo::avatar() const {
    return avatar_;
}

void ZAdminInfo::setCreatedAt(const int64_t value) {
    this->createdAt_ = value;
}

int64_t ZAdminInfo::createdAt() const {
    return createdAt_;
}

void ZAdminInfo::setUpdatedAt(const int64_t value) {
    this->updatedAt_ = value;
}

int64_t ZAdminInfo::updatedAt() const {
    return updatedAt_;
}

void ZAdminInfo::setIsAdmin(const bool value) {
    this->isAdmin_ = value;
}

bool ZAdminInfo::isAdmin() const {
    return isAdmin_;
}

