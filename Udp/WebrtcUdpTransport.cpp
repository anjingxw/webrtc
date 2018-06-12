//
//  WebrtcUdpTransport.cpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/3/2.
//  Copyright © 2018年 infobird. All rights reserved.
//

#include "WebrtcUdpTransport.h"
#include "TransportFactory.h"
#include "rtc_base/bind.h"
#include "rtc_base/byteorder.h"
#include "media/base/rtputils.h"
#include "modules/rtp_rtcp/include/rtp_header_parser.h"

//#include <android/log.h>

WebrtcUdpTransport::WebrtcUdpTransport(TransportType type, std::string  name, bool ipv6):
infobirdTransport_(false),
type_(type),
work_thread_(nullptr),
receive_(nullptr),
transportProxy_(TransportFactory::Instance()->CreateUdpTransport(name, ipv6)){
    transportProxy_->SignalPacketReceived.connect(this, &WebrtcUdpTransport::OnReadPacket);
    localPort =  GetLocalAddress().port();
}

rtc::SocketAddress WebrtcUdpTransport::GetLocalAddress() const{
    return  transportProxy_->GetLocalAddress();
}

bool WebrtcUdpTransport::SetRemoteIPHostPort(const char* ip, uint16_t port){
    return transportProxy_->SetRemoteAddress(rtc::SocketAddress(ip, port));
}
uint16_t WebrtcUdpTransport::GetLocalHostPort(){
    return localPort;
}

bool WebrtcUdpTransport::SetRemoteAddress(const rtc::SocketAddress& dest){
    return transportProxy_->SetRemoteAddress(dest);
}
rtc::SocketAddress WebrtcUdpTransport::GetRemoteAddress() const{
    return transportProxy_->GetRemoteAddress();
};

void WebrtcUdpTransport::SetReceive(rtc::Thread* work_thread, webrtc::PacketReceiver* receive){
    work_thread_ = work_thread;
    receive_ = receive;
}

bool WebrtcUdpTransport::SendRtp(const uint8_t* packet, size_t length, const webrtc::PacketOptions& options){
    //不经过网络测试
//    rtc::CopyOnWriteBuffer buff(packet, length);
//    OnReadPacket(&buff, rtc::PacketTime());
//    return true;
    
    //udp传输方式
    if (infobirdTransport_) {
        uint8_t* sendData = NULL;
        size_t  sendDataLen = 0;
        PreSendProcess(false, (uint8_t*)packet, length, sendData, sendDataLen);
        transportProxy_->SendDataEx((const uint8_t*)sendData, sendDataLen);
        return true;
    }
    
    if (type_ == kVideo) { //测试
        printf("send________%d\n", (int)length);
    }
//
    transportProxy_->SendDataEx(packet, length);
    return true;
}

bool WebrtcUdpTransport::SendRtcp(const uint8_t* packet, size_t length){
    if (type_ == kAudio) { // 音频不发rtcp
        return true;
    }
    transportProxy_->SendDataEx(packet, length);
    return true;
}

void WebrtcUdpTransport::DeliverPacket(rtc::CopyOnWriteBuffer packet, const rtc::PacketTime& packet_time){
    if (receive_ != nullptr) {
        webrtc::MediaType type = webrtc::MediaType::AUDIO;
        if (type_ != kAudio) {
            type = webrtc::MediaType::VIDEO;
        }
        const webrtc::PacketTime webrtc_packet_time(packet_time.timestamp, packet_time.not_before);
        receive_->DeliverPacket(type, packet, webrtc_packet_time);
    }
};


void WebrtcUdpTransport::OnReadPacket(rtc::CopyOnWriteBuffer* packet,  const rtc::PacketTime& packet_time){
    if (work_thread_ == nullptr) {
        return;
    }
    
    if (type_ == kAudio && infobirdTransport_) {
        uint8_t* data = packet->data();
        size_t  len = packet->size();
        if (len != (72-8)) {
            return;
        }
        
        uint8_t rtpData[2048] ={0};
        size_t rtpDataLen = 0;
        PreRecvProcess(false, audioPlayload_, data, len, rtpData, rtpDataLen, seqTemp_);
        
        packet->Clear();
        packet->AppendData(rtpData, rtpDataLen);
        
//        __android_log_print(ANDROID_LOG_ERROR, "MicroMsg***8", "send len = %f, rtpDataLen = %f \n", len+0.0, rtpDataLen+0.0 );
        
        invoker_.AsyncInvoke<webrtc::PacketReceiver::DeliveryStatus>(RTC_FROM_HERE, work_thread_,                                                       rtc::Bind(&WebrtcUdpTransport::DeliverPacket, this, *packet, packet_time));
        return;
    }
    
    if (type_ == kAudio)     { //兼容老版本
        if (!webrtc::RtpHeaderParser::IsRtcp(packet->data(), packet->size())) {
            uint32_t* ssrc = (uint32_t*)(packet->data() + 8);
            *ssrc = htobe32(12345);
            
            uint8_t* playload = (uint8_t*)(packet->data() + 1);
            if (*playload == 18 && packet->size()!=72) {
                return;
            }
            
//            webrtc::RTPHeader header;
//            webrtc::RtpHeaderParser::Create()->Parse(packet->data(), packet->size(), &header);
//            printf("______%d, %d, %d", header.timestamp, (int)packet->size(), header.payloadType);
        }
    }
    
    if (type_ == kVideo) { //测试
        printf("recv________%d\n", (int)packet->size());
    }
    
    invoker_.AsyncInvoke<webrtc::PacketReceiver::DeliveryStatus>(RTC_FROM_HERE, work_thread_,                                                       rtc::Bind(&WebrtcUdpTransport::DeliverPacket, this, *packet, packet_time));
}

void WebrtcUdpTransport::SetInfobirdTransport(bool infobirdTransport){
    infobirdTransport_ = infobirdTransport;
}

void WebrtcUdpTransport::AudioPlayload(int audioPlayload){
    audioPlayload_ = audioPlayload;
}

size_t  WebrtcUdpTransport::PreSendProcess(bool isRtp, uint8_t* rtpData, size_t rtpDataLen, uint8_t*& sendData, size_t& sendDataLen){
    if (isRtp) {
        sendData = rtpData;
        sendDataLen = rtpDataLen;
        return sendDataLen;
    }
    
    uint32_t timeStamp = 0;
    cricket::RtpHeader head;
    if (!cricket::GetRtpHeader(rtpData, rtpDataLen, &head)) {
        sendData = rtpData;
        sendDataLen = rtpDataLen;
        return sendDataLen;
    }
    
    size_t rtpHeadLen= 0 ;
    cricket::GetRtpHeaderLen(rtpData, rtpDataLen, &rtpHeadLen);
    
    //printf("send head.timestamp = %f, head.seq_num = %f \n", head.timestamp+0.0, head.seq_num+0.0 );
    
//     __android_log_print(ANDROID_LOG_ERROR, "MicroMsg***8", "send payload_type = %f, rtpDataLen = %f \n", head.payload_type+0.0, rtpDataLen+0.0 );
    
    if (head.payload_type == 18) { //g729
        timeStamp = (head.timestamp)/480; // ntohl(head.timestamp)/480;
        sendDataLen = rtpDataLen - (rtpHeadLen-4);
        sendData = rtpData + (rtpHeadLen-4);
        *((uint32_t*)sendData) = timeStamp;
        return sendDataLen;
    }
    
    else if(head.payload_type == 102){ //ilbc
        timeStamp = ntohl(head.timestamp)/ 240;
        sendDataLen = rtpDataLen - (rtpHeadLen-4);
        sendData = (rtpData + (rtpHeadLen-4));
        *((uint32_t*)sendData) = timeStamp;
        return sendDataLen;
    }
    else if(head.payload_type ==  103){ //isac
        //no support
    }
    sendData = rtpData;
    sendDataLen = rtpDataLen;
    return sendDataLen;
}

size_t WebrtcUdpTransport::PreRecvProcess(bool isRtp, int payloadType, uint8_t* recvData, size_t recvDataLen, uint8_t* rtpData, size_t& rtpDataLen, int& seq){
    if (isRtp) {
        memcpy(rtpData, recvData, recvDataLen);
        rtpDataLen = recvDataLen;
        return rtpDataLen;
    }
    
    cricket::RtpHeader head;
    head.payload_type = payloadType;
    uint32_t timestamp = *((uint32_t*)recvData);
    head.ssrc = 12345;
    head.seq_num = seq++;
    
    if (head.payload_type == 18) { //g729
        head.timestamp = timestamp *480;
    }
    else if(head.payload_type == 102){ //ilbc
        head.timestamp = timestamp *240;
    }
    
    if ( recvDataLen + 8 > 2048) {
        //error must no arrive
        return 0;
    }
    rtpDataLen = recvDataLen + 8;
    memcpy((rtpData+8), recvData, recvDataLen); //
    uint8_t rtp_data[12];
    rtp_data[0] = 0x80;
    rtp_data[1] = static_cast<uint8_t>(head.payload_type);
    rtp_data[2] = (head.seq_num >> 8) & 0xFF;
    rtp_data[3] = (head.seq_num) & 0xFF;
    rtp_data[4] = head.timestamp >> 24;
    rtp_data[5] = (head.timestamp >> 16) & 0xFF;
    rtp_data[6] = (head.timestamp >> 8) & 0xFF;
    rtp_data[7] = head.timestamp & 0xFF;
    rtp_data[8] = head.ssrc >> 24;
    rtp_data[9] = (head.ssrc >> 16) & 0xFF;
    rtp_data[10] = (head.ssrc >> 8) & 0xFF;
    rtp_data[11] = head.ssrc & 0xFF;
    memcpy(rtpData, rtp_data, 12);
    
    //printf("head.timestamp = %f, head.seq_num = %f \n", head.timestamp+0.0, head.seq_num+0.0 );
    return rtpDataLen;
}

