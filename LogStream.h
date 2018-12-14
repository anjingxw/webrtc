//
//  TestLogStream.hpp
//  webrtcwraper
//
//  Created by anjingxw@126.com on 2018/11/5.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef TestLogStream_hpp
#define TestLogStream_hpp
#include "rtc_base/logging.h"
#include <stdio.h>
#include <string>
#include <memory>
#include <chrono>
#include <iostream>
class LogStream : public rtc::LogSink {
public:
    static void InitLog(const char* path){
        if(s_LogStream == NULL){
            s_LogStream.reset(new LogStream(path));
            //rtc::LogMessage::AddLogToStream(s_LogStream.get(), rtc::INFO);
            rtc::LogMessage::AddLogToStream(s_LogStream.get(), rtc::LS_VERBOSE);    
        }
    }
    
    LogStream(std::string logpath) {
        logpath = logpath +  "/webrtc-native.log";
        _fp = fopen(logpath.c_str(), "w+");
        if (!_fp) {
            return;
        }
        
    }
    virtual ~LogStream() {
        if (_fp) {
            fflush(_fp);
            fclose(_fp);
            _fp = NULL;
        }
    }
    //日志格式可在此接口中定义
    virtual void OnLogMessage(const std::string& message) {
        if (!_fp) {
            return;
        }
        //auto start = std::chrono::system_clock::now();
        //std::cout<<message<<std::endl;
        fwrite(message.c_str(), 1, message.length(), _fp);
        //fflush(_fp);
    }
private:
    FILE* _fp = NULL;
    static std::unique_ptr<LogStream> s_LogStream;
};

#endif /* TestLogStream_hpp */
