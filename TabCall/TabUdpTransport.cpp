//
//  WebrtcUdpTransport.cpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/3/2.
//  Copyright © 2018年 infobird. All rights reserved.
//

#include "TabUdpTransport.h"
#include "TransportFactory.h"
#include "rtc_base/bind.h"
#include "rtc_base/byteorder.h"
#include "media/base/rtputils.h"
#include "modules/rtp_rtcp/include/rtp_header_parser.h"
#include "WebrtcUdpTransport.h"

//#include <android/log.h>
TabAudioUdpTransport::TabAudioUdpTransport(std::string name, bool ipv6):
infobirdTransport_(false),
work_thread_(nullptr),
ipv6_(ipv6){
}

TabAudioUdpTransport::~TabAudioUdpTransport(){
    rtc::CritScope cs(&lock_);
    transportProxys_.clear();
}

bool TabAudioUdpTransport::AddRemoteIPHostPort(const char* ip, uint16_t port){
    rtc::CritScope cs(&lock_);
    rtc::SocketAddress ar(ip, port);
    if(transportProxys_.find(ar) == transportProxys_.end()){
        return false;
    }
    
    std::unique_ptr<UdpTransportProxy> proxy = TransportFactory::Instance()->CreateUdpTransport("", ipv6_);
    proxy->SetRemoteAddress(ar);
    transportProxys_[ar] =  std::shared_ptr<UdpTransportProxy>(proxy.release());
    return true;
}

void TabAudioUdpTransport::DeleteRemoteIPHostPort(const char* ip, uint16_t port){
    rtc::CritScope cs(&lock_);
    transportProxys_.erase(transportProxys_.find(rtc::SocketAddress(ip, port)));
}

void TabAudioUdpTransport::ClearRemoteIPHostPort(){
    rtc::CritScope cs(&lock_);
    transportProxys_.clear();
}

bool TabAudioUdpTransport::SendRtp(const uint8_t* packet, size_t length, const webrtc::PacketOptions& options){
    rtc::CritScope cs(&lock_);
    //udp传输方式
    if (infobirdTransport_) {
        uint8_t* sendData = NULL;
        size_t  sendDataLen = 0;
        WebrtcUdpTransport::PreSendProcess(false, (uint8_t*)packet, length, sendData, sendDataLen);
        for (auto it = transportProxys_.begin(); it != transportProxys_.end(); it++) {
            it->second->SendDataEx((const uint8_t*)sendData, sendDataLen);
        }
        return true;
    }

    for (auto it = transportProxys_.begin(); it != transportProxys_.end(); it++) {
        it->second->SendDataEx(packet, length);
    }
    return true;
}

bool TabAudioUdpTransport::SendRtcp(const uint8_t* packet, size_t length){
    //不发送
    return true;
}

void TabAudioUdpTransport::SetInfobirdTransport(bool infobirdTransport){
    infobirdTransport_ = infobirdTransport;
}

void TabAudioUdpTransport::AudioPlayload(int audioPlayload){
    audioPlayload_ = audioPlayload;
}

