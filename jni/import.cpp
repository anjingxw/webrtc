/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "CallWraperBase.h"
#include "DetectCallWraperBase.h"
#include <jni.h>
extern "C" void
Java_org_webrtc_Logging_nativeEnableLogToDebugOutput(JNIEnv* jni, jclass jcaller, jint nativeSeverity);

extern "C" jlong
Java_org_webrtc_Histogram_nativeCreateCounts(JNIEnv* env, jclass jcaller,
                                             jstring name,
                                             jint min,
                                             jint max,
                                             jint bucketCount);

extern "C" void
Java_org_webrtc_JniCommon_nativeAddRef(JNIEnv* env,
                                       jclass jcaller,
                                       jlong refCountedPointer);

extern "C" void
Java_org_webrtc_NetworkMonitor_nativeNotifyConnectionTypeChanged(JNIEnv* env,
                                                                 jobject jcaller,
                                                                 jlong nativePtr);
extern "C" void
Java_org_webrtc_AudioTrack_nativeSetVolume(JNIEnv* env,
                                                           jclass jcaller,
                                                           jlong track,
                                                           jdouble volume);

extern "C"
jobject Java_org_webrtc_MediaSource_nativeGetState(JNIEnv* env,
                                                              jclass jcaller,
                                                              jlong pointer);

extern "C" jboolean
Java_org_webrtc_MediaStream_nativeAddAudioTrackToNativeStream(JNIEnv* env,
                                                              jclass jcaller,
                                                              jlong stream,
                                                              jlong nativeAudioTrack);

extern "C" jstring
Java_org_webrtc_MediaStreamTrack_nativeGetId(JNIEnv* env,
                                             jclass jcaller,
                                             jlong track);

extern "C" jboolean
Java_org_webrtc_HardwareVideoEncoderFactory_nativeIsSameH264Profile(JNIEnv*
                                                                    env, jclass jcaller,
                                                                    jobject params1,
                                                                    jobject params2);

extern "C" void
Java_org_webrtc_NV12Buffer_nativeCropAndScale(JNIEnv* env,
                                              jclass jcaller,
                                              jint cropX,
                                              jint cropY,
                                              jint cropWidth,
                                              jint cropHeight,
                                              jint scaleWidth,
                                              jint scaleHeight,
                                              jobject src,
                                              jint srcWidth,
                                              jint srcHeight,
                                              jint srcStride,
                                              jint srcSliceHeight,
                                              jobject dstY,
                                              jint dstStrideY,
                                              jobject dstU,
                                              jint dstStrideU,
                                              jobject dstV,
                                              jint dstStrideV);

extern "C" jlong
Java_org_webrtc_VideoDecoderFallback_nativeCreateDecoder(JNIEnv* env, jclass
                                                         jcaller,
                                                         jobject fallback,
                                                         jobject primary);
extern "C" jlong
Java_org_webrtc_VideoEncoderFallback_nativeCreateEncoder(JNIEnv* env, jclass
                                                         jcaller,
                                                         jobject fallback,
                                                         jobject primary);
extern "C" void
Java_org_webrtc_VideoFrame_nativeCropAndScaleI420(JNIEnv* env, jclass
                                                  jcaller,
                                                  jobject srcY,
                                                  jint srcStrideY,
                                                  jobject srcU,
                                                  jint srcStrideU,
                                                  jobject srcV,
                                                  jint srcStrideV,
                                                  jint cropX,
                                                  jint cropY,
                                                  jint cropWidth,
                                                  jint cropHeight,
                                                  jobject dstY,
                                                  jint dstStrideY,
                                                  jobject dstU,
                                                  jint dstStrideU,
                                                  jobject dstV,
                                                  jint dstStrideV,
                                                  jint scaleWidth,
                                                  jint scaleHeight);

extern "C" void Java_org_webrtc_VideoRenderer_nativeCopyPlane(JNIEnv* env, jclass jcaller,
                                                              jobject src,
                                                              jint width,
                                                              jint height,
                                                              jint srcStride,
                                                              jobject dst,
                                                              jint dstStride);

extern "C" void
Java_org_webrtc_VideoSource_nativeAdaptOutputFormat(JNIEnv* env, jclass
                                                    jcaller,
                                                    jlong source,
                                                    jint width,
                                                    jint height,
                                                    jint fps);

extern "C" void
Java_org_webrtc_VideoTrack_nativeAddSink(JNIEnv* env, jclass jcaller,
                                              jlong track,
                                              jlong nativeSink);

extern "C" void
Java_org_webrtc_YuvHelper_nativeI420Copy(JNIEnv* env,
                                         jclass jcaller,
                                         jobject srcY,
                                         jint srcStrideY,
                                         jobject srcU,
                                         jint srcStrideU,
                                         jobject srcV,
                                         jint srcStrideV,
                                         jobject dstY,
                                         jint dstStrideY,
                                         jobject dstU,
                                         jint dstStrideU,
                                         jobject dstV,
                                         jint dstStrideV,
                                         jint width,
                                         jint height);

extern "C" jboolean
Java_org_webrtc_VP9Encoder_nativeIsSupported(JNIEnv* env, jclass jcaller);

extern "C" jlong
Java_org_webrtc_VP8Encoder_nativeCreateEncoder(JNIEnv* env, jclass jcaller);

#define CIWR(rettype, name) \
extern "C" rettype JNIEXPORT JNICALL Java_com_infobird_webrtc_RTC_##name

CIWR(void, noimport)(JNIEnv* env, jclass cls){
    GetCallWraperBase(false);
    GetDetectCallWraperBase();
    SetCallWraperPlatformView(NULL);
    
    Java_org_webrtc_Logging_nativeEnableLogToDebugOutput(env, cls, 0);
    Java_org_webrtc_Histogram_nativeCreateCounts(env, cls, jstring(), 0, 0, 0);
    Java_org_webrtc_JniCommon_nativeAddRef(env, cls, 0);
    Java_org_webrtc_NetworkMonitor_nativeNotifyConnectionTypeChanged(env, NULL, 0);
    Java_org_webrtc_AudioTrack_nativeSetVolume(env, cls, 0, 0.0);
    Java_org_webrtc_MediaSource_nativeGetState(env, cls, 0);
    Java_org_webrtc_MediaStream_nativeAddAudioTrackToNativeStream(env, cls, 0, 0);
    Java_org_webrtc_MediaStreamTrack_nativeGetId(env, cls, 0);
    Java_org_webrtc_HardwareVideoEncoderFactory_nativeIsSameH264Profile(env, cls, 0, 0);
    Java_org_webrtc_NV12Buffer_nativeCropAndScale(env, cls,
                                                  0,
                                                  0,
                                                  0,
                                                  0,
                                                  0,
                                                  0,
                                                  0, //jobject src,
                                                  0,
                                                  0,
                                                  0,
                                                  0,
                                                  0, //jobject dstY,
                                                  0,
                                                  0, //jobject dstU,
                                                  0,
                                                  0, //jobject dstV,
                                                  0);
    Java_org_webrtc_VideoDecoderFallback_nativeCreateDecoder(env, cls,
                                                             0, //jobject fallback,
                                                             0 //jobject primary
                                                             );
    Java_org_webrtc_VideoEncoderFallback_nativeCreateEncoder(env, cls,
                                                             0, //jobject fallback,
                                                             0 //jobject primary
                                                             );
    Java_org_webrtc_VideoFrame_nativeCropAndScaleI420(env, cls,
                                                      0, //jobject srcY,
                                                      0, //jint srcStrideY,
                                                      0, //jobject srcU,
                                                      0, //jint srcStrideU,
                                                      0, //jobject srcV,
                                                      0, //jint srcStrideV,
                                                      0, //jint cropX,
                                                      0, //jint cropY,
                                                      0, //jint cropWidth,
                                                      0, //jint cropHeight,
                                                      0, //jobject dstY,
                                                      0, //jint dstStrideY,
                                                      0, //jobject dstU,
                                                      0, //jint dstStrideU,
                                                      0, //jobject dstV,
                                                      0, //jint dstStrideV,
                                                      0, //jint scaleWidth,
                                                      0 //jint scaleHeight
                                                      );
    Java_org_webrtc_VideoRenderer_nativeCopyPlane(env, cls,
                                                 0, //jobject src,
                                                 0, //jint width,
                                                 0, //jint height,
                                                 0, //jint srcStride,
                                                 0, //jobject dst,
                                                 0 //jint dstStride
                                                 );
    Java_org_webrtc_VideoSource_nativeAdaptOutputFormat(env, cls, 0, 0, 0, 0);
    Java_org_webrtc_VideoTrack_nativeAddSink(env, cls,0, 0);
    Java_org_webrtc_YuvHelper_nativeI420Copy(env, cls,
                                             0, //jobject srcY,
                                             0, //jint srcStrideY,
                                             0, //jobject srcU,
                                             0, //jint srcStrideU,
                                             0, //jobject srcV,
                                             0, //jint srcStrideV,
                                             0, //jobject dstY,
                                             0, //jint dstStrideY,
                                             0, //jobject dstU,
                                             0, //jint dstStrideU,
                                             0, //jobject dstV,
                                             0, //jint dstStrideV,
                                             0, //jint width,
                                             0 //jint height
                                             );
    Java_org_webrtc_VP9Encoder_nativeIsSupported(env, cls);
    Java_org_webrtc_VP8Encoder_nativeCreateEncoder(env, cls);
}


