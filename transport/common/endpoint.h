/**
 * @brief 
 * @date 2025.11.16
 */

#ifndef _ENDPOINT_H_
#define _ENDPOINT_H_

#include "identity.h"
#include "rcmw/config/RoleAttributes.h"
#include <memory>

namespace hnu       {
namespace rcmw      {
namespace transport {

using namespace hnu::rcmw::config;
class Endpoint;
using EndpointPtr = std::shared_ptr<Endpoint>;

class Endpoint {
public:
    explicit Endpoint(const RoleAttributes& attr);
    virtual ~Endpoint();

    const Identity& id() const { return id_; }
    const RoleAttributes& attributes() const { return attr_; }
protected:
    bool enabled_;
    Identity id_;
    RoleAttributes attr_;
};

} // transport
} // rcmw
} // hnu

#endif
