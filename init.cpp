// /**
//  * @brief rcmw初始化
//  * @date 2025.12.27
//  */

// #include "rcmw/init.h"
// #include "rcmw/common/global_data.h"
// #include <string>

// namespace hnu   {
// namespace rcmw  {

// bool Init(const char* binary_name) {
//     std::string logfile_name = (std::string)binary_name + ".log";
//     Logger_Init(logfile_name);

//     auto global_data = common::GlobalData::Instance();
//     return true;
// }

// void Clear() {}

// std::unique_ptr<Node> CreateNode(const std::string& node_name, 
//                                  const std::string& name_space = "") {
//     return std::unique_ptr<Node>(new Node(node_name, name_space));
// }

// } // rcmw
// } // hnu
