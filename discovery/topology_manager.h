/**
 * @brief 拓扑集成管理
 * @date 2025.12.19
 */

#ifndef _DISCOVERY_TOPOLOGY_MANAGER_H_
#define _DISCOVERY_TOPOLOGY_MANAGER_H_

#include "rcmw/discovery/specific_manager/channel_manager.h"
#include "rcmw/discovery/specific_manager/node_manager.h"
#include "rcmw/base/signal_slot.h"
#include "rcmw/config/topology_change.h"
#include "rcmw/transport/rtps/participant.h"
#include "rcmw/discovery/communication/participant_listener.h"
#include "fastrtps/rtps/participant/ParticipantDiscoveryInfo.h"
#include "rcmw/common/macros.h"
#include <functional>
#include <atomic>

namespace hnu       {
namespace rcmw      {
namespace discovery {

using NodeManagerPtr = std::shared_ptr<NodeManager>;
using ChannelManagerPtr = std::shared_ptr<ChannelManager>;

class TopologyManager{
public:
    using ChangeSignal = base::Signal<const ChangeMsg>;
    using ChangeFunc = std::function<void(const ChangeMsg&)>;
    using ChangeConnection = base::Connection<const ChangeMsg>;

    using PartNameContainer = std::map<eprosima::fastrtps::rtps::GUID_t, std::string>;
    using PartInfo = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo;

    virtual ~TopologyManager();

    void Shutdown();

    ChangeConnection AddChangeListener(const ChangeFunc& func);
    void RemoveChangeListener(const ChangeConnection& conn);

    NodeManagerPtr& node_manager() { return node_manager_; }
    ChannelManagerPtr channel_manager() { return channel_manager_; }
private:
    bool Init();
    bool InitNodeManager();
    bool InitChannelManager();

    bool CreateParticipant();
    void OnparticipantChange(const PartInfo& info);
    bool Convert(const PartInfo& info, ChangeMsg* change_msg);
    bool ParseParticipantName(const std::string& participant_name, 
                                std::string* host_name, int* process_id);

    std::atomic<bool> init_;

    NodeManagerPtr node_manager_;
    ChannelManagerPtr channel_manager_;

    transport::ParticipantPtr participant_;

    ParticipantListener* participant_listener_;
    
    ChangeSignal change_signal_;

    PartNameContainer participant_names_;
    DECLARE_SINGLETON(TopologyManager)
};

} // discovery
} // rcmw
} // hnu

#endif
