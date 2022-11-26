//
// Created by curie on 2022/11/6.
//

#ifndef SOCKS5_LOG_HPP
#define SOCKS5_LOG_HPP
#include "string"
#include "../libs/spdlog/include/spdlog/spdlog-inl.h"
using std::string;
bool LOG_INITED = false;
class Log {
public:
    enum Level{
        DEBUG = 0,
        INFO = 1,
        ERROR = 2
    };
    static void init(){
        if(!LOG_INITED) {
            spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%L%$] [%t] %v");
            spdlog::set_level(spdlog::level::level_enum::debug);
            LOG_INITED = true;
        }
    };
    template<typename ...T>
    static void debug(const std::string & tag,const std::string & message ,T...subMessage){
        Log::log("["+tag+"] "+message,Log::Level::DEBUG, subMessage...);
    }
    template<typename ...T>
    static void info(const std::string & tag, const std::string & message ,T...subMessage){
        Log::log("["+tag+"] "+message,Log::Level::INFO, subMessage...);
    }
    template<typename ...T>
    static void error(const std::string & tag, const std::string & message ,T...subMessage){
        Log::log("["+tag+"] "+message,Log::Level::ERROR, subMessage...);
    }
private:
    template<typename ...T>
    static void log(const std::string &message,Level level,T...subMessage){
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
