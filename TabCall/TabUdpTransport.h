//
//  WebrtcUdpTransport.hpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/3/2.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef TabUdpTransport_hpp
#define TabUdpTransport_hpp
#include "api/call/transport.h"
#include "Udp/UdpTransportProxy.h"
#include "call/call.h"
#include "rtc_base/asyncinvoker.h"
#include "rtc_base/sigslot.h"
#include "CallWraperBase.h"
#include <map>

class TabAudioUdpTransport: public webrtc::Transport, public sigslot::has_slots<>{
public:
    TabAudioUdpTransport(std::string name, bool ipv6);
    ~TabAudioUdpTransport();
    bool AddRemoteIPHostPort(const char* ip, uint16_t port);
    void DeleteRemoteIPHostPort(const char* ip, uint16_t port);
    void ClearRemoteIPHostPort();
    
    bool SendRtp(const uint8_t* packet, size_t length, const webrtc::PacketOptions& options) override;
    bool SendRtcp(const uint8_t* packet, size_t length) override;
    
    rtc::SocketAddress GetLocalAddress() const;
public:
    void SetInfobirdTransport(bool infobirdTransport);
    void AudioPlayload(int audioPlayload);
private:
    bool infobirdTransport_;
    int  audioPlayload_;
    int  seqTemp_;
    bool ipv6_;
    static size_t  PreRecvProcess(bool isRtp, int payloadType, uint8_t* recvData, size_t recvDataLen, uint8_t* rtpData, size_t& rtpDataLen, int& seq);
public:
    static size_t  PreSendProcess(bool isRtp, uint8_t* rtpData, size_t rtpDataLen, uint8_t*& sendData, size_t& sendDataLen);
private:
    rtc::Thread* work_thread_;
    rtc::AsyncInvoker invoker_;
    
    std::map<rtc::SocketAddress, std::shared_ptr<UdpTransportProxy>> transportProxys_;
    rtc::CriticalSection lock_;
};
#endif /* WebrtcUdpTransport_hpp */
