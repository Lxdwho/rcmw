/**
 * @brief attributes属性填写类？
 * @date 2025.11.10
 */

#ifndef _ATTRIBUTES_FILLER_H_
#define _ATTRIBUTES_FILLER_H_

#include "fastrtps/rtps/writer/RTPSWriter.h"
#include "fastrtps/rtps/attributes/WriterAttributes.h"
#include "fastrtps/rtps/attributes/ReaderAttributes.h"
#include "fastrtps/rtps/history/ReaderHistory.h"
#include "fastrtps/rtps/history/WriterHistory.h"
#include "fastrtps/qos/WriterQos.h"
#include "fastrtps/qos/ReaderQos.h"
#include "fastrtps/attributes/TopicAttributes.h"
#include "fastrtps/rtps/attributes/HistoryAttributes.h"
#include "rcmw/config/qos_profile.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

namespace hnu       {
namespace rcmw      {
namespace transport {

struct RtpsWriterAttributes {
    HistoryAttributes hatt;
    WriterAttributes watt;
    WriterQos wqos;
    TopicAttributes tatt;
};

struct RtpsReaderAttributes {
    HistoryAttributes hatt;
    ReaderAttributes ratt;
    ReaderQos rqos;
    TopicAttributes tatt;
};

class AttributesFiller {
public:
    AttributesFiller();
    virtual ~AttributesFiller();
    static bool FillInWriterAttr(const std::string& channel_name, 
        const config::QosProfile& qos, RtpsWriterAttributes* writer_attr);
    static bool FillInReaderAttr(const std::string& channel_name,
        const config::QosProfile& qos, RtpsReaderAttributes* reader_attr);
};

} // transport
} // rcmw
} // hnu

#endif
