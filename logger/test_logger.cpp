#include "log.h"
#include <thread>
#include <vector>
#include <unistd.h>

using namespace hnu::rcmw::logger;


int main()
{
	// 单例日志类初始化
	// Logger* log = Logger::Get_instance();
	// log->open("log2.txt");
	// log->level(Logger::LOG_FATAL);
	// log->Set_console(false);
	// log->log(Logger::LOG_INFO, "21212", 12, "%s %s %d", "983123", "983123", 21);

	// log_debug("%s %d", "111111111111111", 22);
	// log_warn("%s %d", "23232323232323", 22);
    
    Logger_Init("log1212.txt");
    usleep(100000);

    auto reader = [&](int id) {
        for (int i = 0; i < 10; ++i) {
            std::cout << "[Thread " << id << "] write log: " << "\n";
            // log_info("Thread, %d-%d", id, i);
			ADEBUG << "Thread-" << id << "--" << i;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    };
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 8; ++i)
        threads.emplace_back(reader, i + 1);

    for (auto& t : threads)
        t.join();
	return 0;
}