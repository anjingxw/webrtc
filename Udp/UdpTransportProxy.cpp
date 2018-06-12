//
//  UdpTransportProxy.cpp
//  webrtc_call_test
//
//  Created by anjingxw@126.com on 2018/3/2.
//  Copyright © 2018年 infobird. All rights reserved.
//

#include "UdpTransportProxy.h"
#include "p2p/base/udptransport.h"
#include "rtc_base/bind.h"


struct MySendPacketMessageData : public rtc::MessageData {
    rtc::CopyOnWriteBuffer packet;
};


UdpTransportProxy::UdpTransportProxy(rtc::Thread* network_thread, cricket::UdpTransport* transport):network_thread_(network_thread), transport_(transport){
    
    transport_->SignalReadyToSend.connect(this, &UdpTransportProxy::OnReadyToSend);
    transport_->SignalReadPacket.connect(this, &UdpTransportProxy::OnReadPacket);
    transport_->SignalNetworkRouteChanged.connect(this, &UdpTransportProxy::OnNetworkRouteChange);
    transport_->SignalWritableState.connect(this, &UdpTransportProxy::OnWritableState);
    transport_->SignalSentPacket.connect(this, &UdpTransportProxy::OnSentPacket);
}

UdpTransportProxy::~UdpTransportProxy(){
    network_thread_->Invoke<void>(RTC_FROM_HERE, rtc::Bind(&UdpTransportProxy::__DeleteUdpTransport, this));
}

rtc::SocketAddress UdpTransportProxy::GetLocalAddress() const{
    if(!network_thread_->IsCurrent()){
        return network_thread_->Invoke<rtc::SocketAddress>(RTC_FROM_HERE, rtc::Bind(&cricket::UdpTransport::GetLocalAddress, transport_.get()));
    }
    return transport_->GetLocalAddress();
}
bool UdpTransportProxy::SetRemoteAddress(const rtc::SocketAddress& dest){
    return network_thread_->Invoke<bool>(RTC_FROM_HERE, rtc::Bind(&cricket::UdpTransport::SetRemoteAddress, transport_.get(), dest));
}

rtc::SocketAddress UdpTransportProxy::GetRemoteAddress() const{
    if(!network_thread_->IsCurrent()){
        return network_thread_->Invoke<rtc::SocketAddress>(RTC_FROM_HERE, rtc::Bind(&cricket::UdpTransport::GetRemoteAddress, transport_.get()));
    }
    return transport_->GetRemoteAddress();
}

rtc::PacketTransportInternal* UdpTransportProxy::GetInternal(){
    return transport_.get();
};

int UdpTransportProxy::SendDataEx(const uint8_t* buff, size_t len){
    if (!network_thread_->IsCurrent()) {
        // Avoid a copy by transferring the ownership of the packet data.
        MySendPacketMessageData* data = new MySendPacketMessageData;
        rtc::CopyOnWriteBuffer packet(buff, len, 2048);
        data->packet = std::move(packet);
        network_thread_->Post(RTC_FROM_HERE, this, 0, data);
        return true;
    }
    return SendData_w(buff, len);
}

int UdpTransportProxy::SendData(const uint8_t* data, size_t len) {
    return network_thread_->Invoke<int>(RTC_FROM_HERE, rtc::Bind(&UdpTransportProxy::SendData_w, this, data, len));
}
int UdpTransportProxy::SendData_w(const uint8_t* data, size_t len) {
    rtc::PacketOptions options;
    return transport_->SendPacket((const char*)data, len, options, 0);
}

void UdpTransportProxy::OnReadyToSend(rtc::PacketTransportInternal* transport){
    
}
void UdpTransportProxy::OnNetworkRouteChange(rtc::Optional<rtc::NetworkRoute> network_route){
    
}
void UdpTransportProxy::OnWritableState(rtc::PacketTransportInternal* packet_transport){
    
}
void UdpTransportProxy::OnSentPacket(rtc::PacketTransportInternal* packet_transport,
                                     const rtc::SentPacket& sent_packet){
    
}

void UdpTransportProxy::__DeleteUdpTransport(){
    transport_.reset();
}

void UdpTransportProxy::OnReadPacket(rtc::PacketTransportInternal* transport,
                  const char* data,
                  size_t len,
                  const rtc::PacketTime& packet_time,
                  int flags){
    
    rtc::CopyOnWriteBuffer packet(data, len);
    SignalPacketReceived(&packet, packet_time);
}

void UdpTransportProxy::OnMessage(rtc::Message* msg){
    MySendPacketMessageData* data = static_cast<MySendPacketMessageData*>(msg->pdata);
    SendData_w(data->packet.data(), data->packet.size());
    delete data;
}
