/**
 * @brief 日志宏
 * @date 2025.11.10
 */

#ifndef _LOG_H_
#define _LOG_H_

#include "logger.h"
#include "logstream.h"

#define Logger_Init(file_name) Logger::Get_instance()->open(file_name)
#define ADEBUG 	Logger::Get_instance()->logStream(Logger::LOG_DEBUG,__FILE__, __LINE__)
#define AINFO 	Logger::Get_instance()->logStream(Logger::LOG_INFO ,__FILE__, __LINE__)
#define AWARN 	Logger::Get_instance()->logStream(Logger::LOG_WARN ,__FILE__, __LINE__)
#define AERROR 	Logger::Get_instance()->logStream(Logger::LOG_ERROR,__FILE__, __LINE__)
#define AFATAL 	Logger::Get_instance()->logStream(Logger::LOG_FATAL,__FILE__, __LINE__)

#define log_debug(format, ...) Logger::Get_instance()->log(Logger::LOG_DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_info(format, ...)  Logger::Get_instance()->log(Logger::LOG_INFO , __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_warn(format, ...)  Logger::Get_instance()->log(Logger::LOG_WARN , __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_error(format, ...) Logger::Get_instance()->log(Logger::LOG_ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define log_fatal(format, ...) Logger::Get_instance()->log(Logger::LOG_FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)

#endif
