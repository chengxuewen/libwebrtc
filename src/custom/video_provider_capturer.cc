#include "video_provider_capturer.h"

#include "src/internal/video_capturer.h"
#include "src/rtc_video_frame_impl.h"

namespace libwebrtc {

class Capturer : public webrtc::internal::VideoCapturer,
                 public RTCVideoRenderer<scoped_refptr<RTCVideoFrame>> {
 public:
  Capturer(scoped_refptr<RTCVideoProvider> videoProvider)
      : videoProvider_(videoProvider) {}
  ~Capturer() override { this->StopCapture(); }

  bool StartCapture() override {
    if (!running_.exchange(true)) {
      videoProvider_->AddRenderer(this);
    }
    return true;
  }
  bool CaptureStarted() override { return running_.load(); }
  void StopCapture() override {
    if (running_.exchange(false)) {
      videoProvider_->RemoveRenderer(this);
    }
  }

  void OnFrame(scoped_refptr<RTCVideoFrame> frame) override {
    if (running_.load()) {
      const auto now = std::chrono::steady_clock::now();
      const auto usecs = std::chrono::duration_cast<std::chrono::microseconds>(
                             now.time_since_epoch())
                             .count();
      VideoFrameBufferImpl* frame_impl =
          static_cast<VideoFrameBufferImpl*>(frame.get());
      auto buffer = frame_impl->buffer();
      webrtc::internal::VideoCapturer::OnFrame(
          webrtc::VideoFrame::Builder()
              .set_video_frame_buffer(buffer)
              .set_rotation(webrtc::VideoRotation::kVideoRotation_0)
              .set_timestamp_us(usecs)
              .set_id(id_++)
              .build());
    }
  }

 private:
  uint16_t id_{0};
  std::atomic<bool> running_{false};
  scoped_refptr<RTCVideoProvider> videoProvider_;
};

class RTCVideoProviderPrivate {
 public:
  explicit RTCVideoProviderPrivate(RTCVideoProvider* p) : q_ptr(p) {}
  virtual ~RTCVideoProviderPrivate() {}

  RTCVideoProvider* const q_ptr;

  std::mutex mutex_;
  std::list<RTCVideoRenderer<scoped_refptr<RTCVideoFrame>>*> renderers_;
};

RTCVideoProvider::RTCVideoProvider()
    : impl_(new RTCVideoProviderPrivate(this)) {}

RTCVideoProvider::~RTCVideoProvider() {
  std::unique_lock<std::mutex> lock(impl_->mutex_);
  impl_->renderers_.clear();
  delete impl_;
}

void RTCVideoProvider::AddRenderer(
    RTCVideoRenderer<scoped_refptr<RTCVideoFrame>>* renderer) {
  std::unique_lock<std::mutex> lock(impl_->mutex_);
  impl_->renderers_.push_back(renderer);
}

void RTCVideoProvider::RemoveRenderer(
    RTCVideoRenderer<scoped_refptr<RTCVideoFrame>>* renderer) {
  std::unique_lock<std::mutex> lock(impl_->mutex_);
  impl_->renderers_.erase(
      std::remove_if(
          impl_->renderers_.begin(), impl_->renderers_.end(),
          [renderer](
              const RTCVideoRenderer<scoped_refptr<RTCVideoFrame>>* renderer_) {
            return renderer_ == renderer;
          }),
      impl_->renderers_.end());
}

void RTCVideoProvider::push(scoped_refptr<RTCVideoFrame> video_frame) {
  std::unique_lock<std::mutex> lock(impl_->mutex_);
  for (auto renderer : impl_->renderers_) {
    renderer->OnFrame(video_frame);
  }
}

VideoProviderCapturer::VideoProviderCapturer(
    scoped_refptr<RTCVideoProvider> videoProvider)
    : RTCVideoCapturerImpl(std::make_shared<Capturer>(videoProvider)) {}

scoped_refptr<RTCVideoCapturer> RTCVideoProvider::CreateCapturer(
    scoped_refptr<RTCVideoProvider> provider) {
  return scoped_refptr<RTCVideoCapturer>(
      new RefCountedObject<VideoProviderCapturer>(provider));
}

}  // namespace libwebrtc