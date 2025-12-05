#ifndef LIB_WEBRTC_RTC_VIDEO_SINK_ADPTER_HXX
#define LIB_WEBRTC_RTC_VIDEO_SINK_ADPTER_HXX

#include <list>
#include <mutex>

#include "rtc_types.h"
#include "rtc_video_device.h"
#include "rtc_video_frame.h"
#include "rtc_video_renderer.h"

namespace libwebrtc {

class RTCVideoProviderPrivate;
class LIB_WEBRTC_API RTCVideoProvider : public RefCountInterface {
 public:
  RTCVideoProvider();
  ~RTCVideoProvider();

  static scoped_refptr<RTCVideoProvider> Create() {
    return scoped_refptr<RTCVideoProvider>(
        new RefCountedObject<RTCVideoProvider>());
  }
  static scoped_refptr<RTCVideoCapturer> CreateCapturer(
      scoped_refptr<RTCVideoProvider> provider);

  void AddRenderer(RTCVideoRenderer<scoped_refptr<RTCVideoFrame>>* renderer);

  void RemoveRenderer(RTCVideoRenderer<scoped_refptr<RTCVideoFrame>>* renderer);

  void push(scoped_refptr<RTCVideoFrame> video_frame);

  void push(int width, int height, const uint8_t* buffer, int length) {
    this->push(RTCVideoFrame::Create(width, height, buffer, length));
  }

  void push(int width, int height, const uint8_t* data_y, int stride_y,
            const uint8_t* data_u, int stride_u, const uint8_t* data_v,
            int stride_v) {
    this->push(RTCVideoFrame::Create(width, height, data_y, stride_y, data_u,
                                     stride_u, data_v, stride_v));
  }

 private:
  friend class RTCVideoProviderPrivate;
  RTCVideoProviderPrivate* impl_;
};

}  // namespace libwebrtc

#endif  // LIB_WEBRTC_RTC_VIDEO_SINK_ADPTER_HXX
