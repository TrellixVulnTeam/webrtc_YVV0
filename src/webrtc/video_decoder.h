/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_VIDEO_DECODER_H_
#define WEBRTC_VIDEO_DECODER_H_

#include <vector>

#include "webrtc/common_types.h"
#include "webrtc/typedefs.h"
#include "webrtc/video_frame.h"

namespace webrtc {

class RTPFragmentationHeader;
// TODO(pbos): Expose these through a public (root) header or change these APIs.
struct CodecSpecificInfo;
struct VideoCodec;

class DecodedImageCallback {
 public:
  virtual ~DecodedImageCallback() {}

  virtual int32_t Decoded(I420VideoFrame& decodedImage) = 0;
  virtual int32_t ReceivedDecodedReferenceFrame(const uint64_t pictureId) {
    return -1;
  }

  virtual int32_t ReceivedDecodedFrame(const uint64_t pictureId) { return -1; }
};

class VideoDecoder {
 public:
  enum DecoderType {
    kVp8,
    kVp9,
    kUnsupportedCodec,
  };

  static VideoDecoder* Create(DecoderType codec_type);

  virtual ~VideoDecoder() {}

  virtual int32_t InitDecode(const VideoCodec* codec_settings,
                             int32_t number_of_cores) = 0;

  virtual int32_t Decode(const EncodedImage& input_image,
                         bool missing_frames,
                         const RTPFragmentationHeader* fragmentation,
                         const CodecSpecificInfo* codec_specific_info = NULL,
                         int64_t render_time_ms = -1) = 0;

  virtual int32_t RegisterDecodeCompleteCallback(
      DecodedImageCallback* callback) = 0;

  virtual int32_t Release() = 0;
  virtual int32_t Reset() = 0;

  virtual int32_t SetCodecConfigParameters(const uint8_t* /*buffer*/,
                                           int32_t /*size*/) {
    return -1;
  }

  virtual VideoDecoder* Copy() { return NULL; }
};

// Class used to wrap external VideoDecoders to provide a fallback option on
// software decoding when a hardware decoder fails to decode a stream due to
// hardware restrictions, such as max resolution.
class VideoDecoderSoftwareFallbackWrapper : public webrtc::VideoDecoder {
 public:
  VideoDecoderSoftwareFallbackWrapper(VideoCodecType codec_type,
                                      VideoDecoder* decoder);

  int32_t InitDecode(const VideoCodec* codec_settings,
                     int32_t number_of_cores) override;

  int32_t Decode(const EncodedImage& input_image,
                 bool missing_frames,
                 const RTPFragmentationHeader* fragmentation,
                 const CodecSpecificInfo* codec_specific_info,
                 int64_t render_time_ms) override;

  int32_t RegisterDecodeCompleteCallback(
      DecodedImageCallback* callback) override;

  int32_t Release() override;
  int32_t Reset() override;

 private:
  bool InitFallbackDecoder();

  const DecoderType decoder_type_;
  VideoDecoder* const decoder_;

  VideoCodec codec_settings_;
  int32_t number_of_cores_;
  rtc::scoped_ptr<VideoDecoder> fallback_decoder_;
  DecodedImageCallback* callback_;
};

}  // namespace webrtc

#endif  // WEBRTC_VIDEO_DECODER_H_
