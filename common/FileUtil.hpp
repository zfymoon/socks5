//
// Created by curie on 2022/11/11.
//

#ifndef SOCKS5_FILEUTIL_HPP
#define SOCKS5_FILEUTIL_HPP

#include "string"
#include "fstream"
#include "Log.hpp"
using std::string;
using std::ifstream;

namespace FileUtil {

    const string TAG = "FileUtil";

    string readContent(const string & fileName){
        if(fileName.empty()){
            Log::error(TAG,"File name is empty");
            return "";
        } else{
            ifstream inputStream;
            inputStream.open(fileName, ifstream::in);
            if(inputStream){
                string result;
                string tmp;
                while(getline(inputStream,tmp)){
                    result += (tmp + "\n");
                }
                result.pop_back();
                return result;
            } else{
                Log::error(TAG,"Open failed: {}",fileName);
                return "";
            }
        }
    }
};


#endif //SOCKS5_FILEUTIL_HPP
