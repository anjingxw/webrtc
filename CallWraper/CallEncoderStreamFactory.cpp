//
//  CallEncoderStreamFactory.cpp
//  webrtcwraper
//
//  Created by anjingxw@126.com on 2018/11/13.
//  Copyright © 2018年 infobird. All rights reserved.
//

#include "CallEncoderStreamFactory.hpp"


CallEncoderStreamFactory::CallEncoderStreamFactory(std::string codec_name)
: codec_name_(codec_name),
max_qp_(51),
max_framerate_(30),
is_screencast_(false),
conference_mode_(false) {}

static int GetMaxDefaultVideoBitrateKbps(int width, int height) {
    if (width * height <= 320 * 240) {
        return 600;
    } else if (width * height <= 640 * 480) {
        return 1700;
    } else if (width * height <= 960 * 540) {
        return 2000;
    } else {
        return 2500;
    }
}

std::vector<webrtc::VideoStream> CallEncoderStreamFactory::CreateEncoderStreams(
                                                                            int width,
                                                                            int height,
                                                                            const webrtc::VideoEncoderConfig& encoder_config) {
    // For unset max bitrates set default bitrate for non-simulcast.
    width =  240;
    height = 320;
    
    int max_bitrate_bps =
    (encoder_config.max_bitrate_bps > 0)
    ? encoder_config.max_bitrate_bps
    : GetMaxDefaultVideoBitrateKbps(width, height) * 1000;
    
    webrtc::VideoStream stream;
    stream.width = width;
    stream.height = height;
    stream.max_framerate = max_framerate_;
    stream.min_bitrate_bps = encoder_config.min_transmit_bitrate_bps;
    stream.target_bitrate_bps = stream.max_bitrate_bps = max_bitrate_bps;
    stream.max_qp = max_qp_;
    stream.bitrate_priority = encoder_config.bitrate_priority;

    
    std::vector<webrtc::VideoStream> streams;
    streams.push_back(stream);
    return streams;
}
