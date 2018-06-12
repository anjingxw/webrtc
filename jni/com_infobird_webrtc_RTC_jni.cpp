//
//  service_jni.cpp
//  qtbservice
//
//  Created by zzh on 16/4/18.
//
//
#include <jni.h>
#include <android/log.h>
#include "sdk/android/src/jni/pc/video.h"
#include "sdk/android/src/jni/jni_generator_helper.h"
#include "sdk/android/src/jni/jni_helpers.h"
#include "api/mediaconstraintsinterface.h"
#include "api/mediastreamproxy.h"
#include "api/mediastreamtrackproxy.h"
#include "pc/videotrack.h"
#include "CallWraper/CallWraperFactory.h"
#include "CallWraper/CallWraperPlatform.h"

#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "media/engine/webrtcvideodecoderfactory.h"
#include "media/engine/webrtcvideoencoderfactory.h"
#include "media/base/mediaengine.h"
#include "sdk/android/src/jni/pc/media.h"
#include "sdk/android/src/jni/pc/video.h"
#include "api/videosinkinterface.h"
#include "api/videosourceinterface.h"
#include "modules/utility/include/jvm_android.h"
#include "sdk/android/src/jni/videoframe.h"
#include "sdk/android/src/jni/androidvideotracksource.h"

#include "base/logging.h"

#define CIWR(rettype, name) \
extern "C" rettype JNIEXPORT JNICALL Java_com_infobird_webrtc_RTC_##name
static bool factory_static_initialized = false;
static bool video_hw_acceleration_enabled = true;

rtc::scoped_refptr<webrtc::VideoTrackInterface> CreateVideoTrack_w(std::string id,  webrtc::VideoTrackSourceInterface* source){
    rtc::scoped_refptr<webrtc::VideoTrackInterface> track(webrtc::VideoTrack::Create(id, source, CallWraperFactory::Instance()->work_thread()));
    return webrtc::VideoTrackProxy::Create(CallWraperFactory::Instance()->signaling_thread(), CallWraperFactory::Instance()->work_thread(), track);
}

void CreateVideoEncoderFactory_w(JNIEnv* env, const base::android::JavaParamRef<jobject>& jencoder_factory, const base::android::JavaParamRef<jobject>& local_egl_context
                                 ){
    cricket::WebRtcVideoEncoderFactory*  legacy_video_encoder_factory = nullptr;
    std::unique_ptr<webrtc::VideoEncoderFactory> video_encoder_factory = nullptr;
    if (jencoder_factory.is_null()) {
        legacy_video_encoder_factory = webrtc::jni::CreateLegacyVideoEncoderFactory();
        video_encoder_factory = std::unique_ptr<webrtc::VideoEncoderFactory>(webrtc::jni::WrapLegacyVideoEncoderFactory(legacy_video_encoder_factory));
    } else
    {
        video_encoder_factory = std::unique_ptr<webrtc::VideoEncoderFactory>(webrtc::jni::CreateVideoEncoderFactory(env, jencoder_factory));
    }
    webrtc::jni::SetEglContext(env, legacy_video_encoder_factory, local_egl_context);
    CallWraperPlatform::CreateVideoEncoderFactory(std::move(video_encoder_factory));
}

void CreateVideoDecoderFactory_w(JNIEnv* env, const base::android::JavaParamRef<jobject>& jdecoder_factory, const base::android::JavaParamRef<jobject>& remote_egl_context){
    cricket::WebRtcVideoDecoderFactory* legacy_video_decoder_factory = nullptr;
    std::unique_ptr<webrtc::VideoDecoderFactory> video_decoder_factory = nullptr;
    if (jdecoder_factory.is_null()) {
        legacy_video_decoder_factory = webrtc::jni::CreateLegacyVideoDecoderFactory();
        video_decoder_factory = std::unique_ptr<webrtc::VideoDecoderFactory>(webrtc::jni::WrapLegacyVideoDecoderFactory(legacy_video_decoder_factory));
    } else
    {
        video_decoder_factory = std::unique_ptr<webrtc::VideoDecoderFactory>(webrtc::jni::CreateVideoDecoderFactory(env, jdecoder_factory));
    }
    webrtc::jni::SetEglContext(env, legacy_video_decoder_factory, remote_egl_context);
    CallWraperPlatform::CreateVideoDecoderFactory(std::move(video_decoder_factory));
}

CIWR(jlong, nativeCreateVideoSource)(JNIEnv* env,
                        jclass jcaller,
                        jobject surfaceTextureHelper,
                        jboolean is_screencast) {
    
    void * videoTrackSource = CreateVideoSource(env,
                                                CallWraperFactory::Instance()->signaling_thread(),
                                                CallWraperFactory::Instance()->work_thread(),
                                                base::android::JavaParamRef<jobject>(env, surfaceTextureHelper),
                                                is_screencast);
    
    return webrtc::jni::jlongFromPointer(videoTrackSource);
}

CIWR(jlong, nativeCreateVideoTrack)(JNIEnv* env,
                                    jclass jcaller,
                                    jstring id,
                                    jlong nativeVideoSource) {
    std::string sid = webrtc::jni::JavaToStdString(env, id);
    webrtc::VideoTrackSourceInterface* source = reinterpret_cast<webrtc::VideoTrackSourceInterface*>(nativeVideoSource);
    
    rtc::Thread* signaling_thread =  CallWraperFactory::Instance()->signaling_thread();
    rtc::scoped_refptr<webrtc::VideoTrackInterface> track = signaling_thread->Invoke<rtc::scoped_refptr<webrtc::VideoTrackInterface>>(RTC_FROM_HERE, std::bind(CreateVideoTrack_w,sid,  source));
    
    CallWraperPlatform::source_ = track.get();
    return webrtc::jni::jlongFromPointer(track.release());
}

CIWR(void, nativeSetRemoteRender)(JNIEnv* env, jclass jcaller, jlong render_){
   CallWraperPlatform::render_ = reinterpret_cast<rtc::VideoSinkInterface<webrtc::VideoFrame>*>(render_);
}

CIWR(void, nativeCreateVideoEncoderFactory)(JNIEnv* env,
                                            jclass jcaller,
                                            jobject encoderFactory,
                                            jobject localEGLContext){
    CreateVideoEncoderFactory_w(env, base::android::JavaParamRef<jobject>(env, encoderFactory), base::android::JavaParamRef<jobject>(env, localEGLContext));
}


CIWR(void, nativeCreateVideoDecoderFactory)(JNIEnv* env,
                                              jclass jcaller,
                                              jobject decoderFactory,
                                              jobject remoteEGLContext){
    CreateVideoDecoderFactory_w(env, base::android::JavaParamRef<jobject>(env, decoderFactory), base::android::JavaParamRef<jobject>(env, remoteEGLContext));
}

CIWR(void, nativeInitializeAndroidGlobals)(JNIEnv* env, jclass jcaller,
                                       jobject context,
                                       jboolean videoHwAcceleration) {
    video_hw_acceleration_enabled = videoHwAcceleration;
    if (!factory_static_initialized) {
        webrtc::JVM::Initialize(webrtc::jni::GetJVM());
        factory_static_initialized = true;
    }
}

CIWR(void, nativeReleaseVideoEncoderFactory)(JNIEnv* env, jclass jcaller){
    CallWraperPlatform::CreateVideoEncoderFactory(nullptr);
}

CIWR(void, nativeReleaseVideoDecoderFactory)(JNIEnv* env, jclass jcaller){
    CallWraperPlatform::CreateVideoDecoderFactory(nullptr);
}

CIWR(jlong, CreateCallWraper)(JNIEnv* env, jclass jcaller, jboolean onlyAudio) {
    CallWraper*  _callWraper = CallWraperFactory::Instance()->PCreateCallWraper(false);
    _callWraper->Config(onlyAudio, "g729", true);
    _callWraper->CreateCallAndAudioDevice();
    
    WebrtcUdpTransportBase* audioTransport = _callWraper->AudioTransport();
    audioTransport->SetInfobirdTransport(true);
    //rtc::SocketAddress addr = audioTransport->GetLocalAddress();
    //addr.SetIP("127.0.0.1");
    audioTransport->SetRemoteIPHostPort("127.0.0.1", audioTransport->GetLocalHostPort());
    
    WebrtcUdpTransportBase* videoTransport = _callWraper->VideoTransport();
    videoTransport->SetRemoteIPHostPort("127.0.0.1", videoTransport->GetLocalHostPort());
    return webrtc::jni::jlongFromPointer(_callWraper);
}

CIWR(void, ReleaseCallWraper)(JNIEnv* env, jclass jcaller, jlong callwraper) {
    CallWraper* _callWraper = reinterpret_cast<CallWraper*>(callwraper);
    delete _callWraper;
}

CIWR(void, CallWraperStart)(JNIEnv* env, jclass jcaller, jlong callwraper) {
    CallWraper* _callWraper = reinterpret_cast<CallWraper*>(callwraper);
    _callWraper->Start();
}

CIWR(void, CallWraperStop)(JNIEnv* env, jclass jcaller, jlong callwraper) {
    CallWraper* _callWraper = reinterpret_cast<CallWraper*>(callwraper);
    _callWraper->Stop();
}

CIWR(void, CallWraperStartOnlySend) (JNIEnv* env, jclass jcaller, jlong callwraper) {
    CallWraper* _callWraper = reinterpret_cast<CallWraper*>(callwraper);
    _callWraper->StartOnlySend();
}

CIWR(void, CallWraperStartOnlyRecv)(JNIEnv* env, jclass jcaller, jlong callwraper) {
    CallWraper* _callWraper = reinterpret_cast<CallWraper*>(callwraper);
    _callWraper->StartOnlyRecv();
}

