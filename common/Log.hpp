//
// Created by curie on 2022/11/6.
//

#ifndef SOCKS5_LOG_HPP
#define SOCKS5_LOG_HPP
#include "string"
#include "../libs/spdlog/include/spdlog/spdlog-inl.h"
#include "mutex"

using std::string_view;
using std::string;
using std::once_flag;

once_flag initedFlag;
class Log {
public:
    enum Level{
        DEBUG = 0,
        INFO = 1,
        ERROR = 2
    };
    static void init(){
        std::call_once(initedFlag, initLog);
    };
    template<typename ...T>
    static void debug(const std::string_view & tag,const std::string & message ,T...subMessage){
        Log::log("["+string(tag)+"] "+message,Log::Level::DEBUG, subMessage...);
    }
    template<typename ...T>
    static void info(const std::string_view & tag, const std::string & message ,T...subMessage){
        Log::log("["+string(tag)+"] "+message,Log::Level::INFO, subMessage...);
    }
    template<typename ...T>
    static void error(const std::string_view & tag, const std::string & message ,T...subMessage){
        Log::log("["+string(tag)+"] "+message,Log::Level::ERROR, subMessage...);
    }
private:
    static void initLog(){
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%L%$] [%t] %v");
        spdlog::set_level(spdlog::level::level_enum::debug);
    }
    template<typename ...T>
    static void log(const std::string_view &message,Level level,T...subMessage){
        spdlog::level::level_enum realLevel;
        if(level == Level::ERROR){
            realLevel = spdlog::level::level_enum::err;
        } else if(level == Level::DEBUG){
            realLevel = spdlog::level::level_enum::debug;
        } else {
            realLevel = spdlog::level::level_enum::info;
        }
        spdlog::log(realLevel, message, subMessage...);
    };
};
#endif //SOCKS5_LOG_HPP
