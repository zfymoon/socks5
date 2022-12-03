//
// Created by curie on 2022/11/26.
//

#ifndef SOCKS5_CONFIG_HPP
#define SOCKS5_CONFIG_HPP

#include <string>
#include <unordered_map>
#include <optional>
#include "./User.hpp"
#include "../../common/FileUtil.hpp"
#include "../../libs/json/nlohmann/json.hpp"
#include "../../common/Log.hpp"
#include "./Server.hpp"
using std::unordered_map;
using std::string;
using std::optional;
using nlohmann::json;

class Config {
public:
    const string TAG = "Config";
    int port {};
    bool auth {};
    int serverBufferSize {};
    int clientBufferSize {};
    Server forwardServer {};
    unordered_map<string,User> userMap;
    bool forwardEnable {};
    explicit Config(const string & configPath){
        string configContent = FileUtil::readContent(configPath);
        if(!configContent.empty()){
            json configJsonObj = json::parse(configContent);
            if(!configJsonObj.empty()){
                if( configJsonObj.contains("port") && configJsonObj["port"].is_number_integer()){
                    port = configJsonObj["port"].get<int>();
                }
                if( configJsonObj["client_buffer_size"].is_number_integer()){
                    clientBufferSize = configJsonObj["client_buffer_size"].get<int>();
                }
                if( configJsonObj["server_buffer_size"].is_number_integer()){
                    serverBufferSize = configJsonObj["client_buffer_size"].get<int>();
                }
                if( configJsonObj["auth"].is_boolean()){
                    auth= configJsonObj["auth"].get<bool>();
                }
                if( configJsonObj.contains("user")){
                    for( auto & item: configJsonObj["user"]){
                        if(item.contains("name") ){
                            User user {item["name"].get<string>(),item["token"].get<string>()};
                            if(user.token.length() >= USER_TOKEN_MIN){
                                userMap.emplace(item["name"].get<string>(),user);
                            }else {
                                Log::error(TAG,"User[{}] token is less than {}. Please modify.", user.name, USER_TOKEN_MIN);
                            }
                        }
                    }
                }
                //forward server
                if( configJsonObj.contains("forward")){
                    auto forwardConfig = configJsonObj["forward"].get<json>();
                    if( forwardConfig["enable"].is_boolean()){
                        forwardEnable = forwardConfig["enable"].get<bool>();
                    }
                    if( forwardConfig["host"].is_string()){
                        forwardServer.server = forwardConfig["host"].get<string>();
                    }
                    if( forwardConfig["port"].is_number_integer()){
                        forwardServer.port = forwardConfig["port"].get<int>();
                    }
                    if( forwardConfig.contains("user")){
                        if(forwardConfig["user"]["name"].is_string() ){
                            User user {forwardConfig["user"]["name"].get<string>(),forwardConfig["user"]["token"].get<string>()};
                            if(user.token.length() >= USER_TOKEN_MIN){
                                forwardServer.user = user;
                            }else {
                                Log::error(TAG,"User[{}] token is less than {}. Please modify.", user.name, USER_TOKEN_MIN);
                            }
                        }
                    }
                }
            }
        }
    }
    optional<User> getUser(const string & name){
        if(!name.empty()){
            return userMap.at(name);
        } else{
            return std::nullopt;
        }
    }
};


#endif //SOCKS5_CONFIG_HPP
