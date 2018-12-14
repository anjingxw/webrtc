//
//  CallEncoderStreamFactory.hpp
//  webrtcwraper
//
//  Created by anjingxw@126.com on 2018/11/13.
//  Copyright © 2018年 infobird. All rights reserved.
//

#ifndef CallEncoderStreamFactory_hpp
#define CallEncoderStreamFactory_hpp
#include "api/call/transport.h"
#include "api/optional.h"
#include "api/video/video_frame.h"
#include "api/video_codecs/sdp_video_format.h"
#include "api/videosinkinterface.h"
#include "api/videosourceinterface.h"
#include "call/call.h"
#include "call/flexfec_receive_stream.h"
#include "call/video_receive_stream.h"
#include "call/video_send_stream.h"

class CallEncoderStreamFactory
: public webrtc::VideoEncoderConfig::VideoStreamFactoryInterface {
public:
    CallEncoderStreamFactory(std::string codec_name);
    
private:
    std::vector<webrtc::VideoStream> CreateEncoderStreams(
                                                          int width,
                                                          int height,
                                                          const webrtc::VideoEncoderConfig& encoder_config) override;
    
    const std::string codec_name_;
    const int max_qp_;
    const int max_framerate_;
    const bool is_screencast_;
    const bool conference_mode_;
};

#endif /* CallEncoderStreamFactory_hpp */
