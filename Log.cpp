//
// Created by fatjoker on 11.05.19.
//

#include "Log.h"

std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

void Log::init() {
    s_ClientLogger = spdlog::basic_logger_mt("client_logger", "client_logs.txt");
    s_ClientLogger->flush_on(spdlog::level::err);


}
