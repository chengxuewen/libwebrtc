#ifndef _VIDEO_PROVIDER_CAPTURER_H_
#define _VIDEO_PROVIDER_CAPTURER_H_

#include <chrono>

#include "rtc_video_provider.h"
#include "src/rtc_video_device_impl.h"

namespace libwebrtc {

class VideoProviderCapturer : public RTCVideoCapturerImpl {
 public:
  VideoProviderCapturer(scoped_refptr<RTCVideoProvider> videoProvider);
  ~VideoProviderCapturer() override {}
};

}  // namespace libwebrtc

#endif  // _VIDEO_PROVIDER_CAPTURER_H_
