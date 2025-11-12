/**
 * @brief 
 * @date 2025.11.12
 */

#ifndef _IDENTITY_H_
#define _IDENTITY_H_

#include <string>
#include <memory>
#include <cstring>

namespace hnu       {
namespace rcmw      {
namespace transport {

constexpr uint8_t ID_SIZE = 8;

/* 标识符类 */
class Identity
{
private:
    void Update();          // 标识符更新
    char data_[ID_SIZE];    // 标识符存储
    uint64_t hash_value_;   // 标识符哈希值
public:
    explicit Identity(bool need_generate = true);
    Identity(const Identity& other);
    virtual ~Identity();

    Identity& operator=(const Identity& other);
    bool operator==(const Identity& other) const;
    bool operator!=(const Identity& other) const;

    /*  */
    std::string ToString() const;
    size_t Length() const;
    uint64_t HashValue() const;
    const char* data() const { return data_; }

    void set_data(const char* data) {
        if(data == nullptr) return;
        std::memcpy(data_, data, sizeof(data_));
        Update();
    }
};

}
}
}

#endif
