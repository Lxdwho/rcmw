/**
 * @brief 标识符类实现
 * @date 2025.11.12
 */

#include "identity.h"
#include "rcmw/common/util.h"
#include <uuid/uuid.h>

namespace hnu       {
namespace rcmw      {
namespace transport {

void Identity::Update() {
    hash_value_ = common::Hash(std::string(data_, ID_SIZE));
}

Identity::Identity(bool need_generate = true) : hash_value_(0) {
    std::memset(data_, 0, ID_SIZE);
    if(need_generate) {
        uuid_t uuid;
        uuid_generate(uuid);
        std::memcpy(data_, uuid, ID_SIZE);
        Update();
    }
}

Identity::Identity(const Identity& other) {
    std::memcpy(data_, other.data_, ID_SIZE);
    hash_value_ = other.hash_value_;
}

Identity::~Identity() {}


Identity& Identity::operator=(const Identity& other) {
    if(this != &other) {
        std::memcpy(data_, other.data_, ID_SIZE);
        hash_value_ = other.hash_value_;
    }
    return *this;
}

bool Identity::operator==(const Identity& other) const {
    return std::memcmp(data_, other.data_, ID_SIZE);
}

bool Identity::operator!=(const Identity& other) const {
    return !std::memcmp(data_, other.data_, ID_SIZE);
}

std::string Identity::ToString() const {
    return std::to_string(hash_value_);
}

size_t Identity::Length() const {
    return ID_SIZE;
}

uint64_t Identity::HashValue() const {
    return hash_value_;
}

} // transport
} // rcmw
} // hnu
