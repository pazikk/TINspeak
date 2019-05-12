//
// Created by fatjoker on 11.05.19.
//

#ifndef CLION_LOG_H
#define CLION_LOG_H

#include <spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <iostream>
class Log {
public:
    static void init();
    inline static std::shared_ptr<spdlog::logger>& GetClientLogger() {return s_ClientLogger;}


private:
    static std::shared_ptr<spdlog::logger> s_ClientLogger;

};

#define CLIENT_INFO(...) ::Log::GetClientLogger()->info(__VA_ARGS__)
#define CLIENT_WARN(...) ::Log::GetClientLogger()->warn(__VA_ARGS__)
#define CLIENT_ERROR(...) ::Log::GetClientLogger()->error(__VA_ARGS__)
#define CLIENT_CRITICAL(...) ::Log::GetClientLogger()->critical(__VA_ARGS__)

#endif //CLION_LOG_H
