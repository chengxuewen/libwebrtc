#include "libwebrtc.h"

#include <rtc_base/logging.h>
#include "api/scoped_refptr.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/thread.h"
#include "rtc_peerconnection_factory_impl.h"

namespace libwebrtc {
    static LibWebRTC::LogCallback rtcLogCallback = NULL;
    class WebRTCRedirectLogSink : virtual public rtc::LogSink
    {
    public:
        void OnLogMessage(const std::string &message)
        {
        }
        void OnLogMessage(const rtc::LogLineRef &line)
        {
            const auto threadId = line.thread_id().has_value() ? std::to_string(line.thread_id().value()) : "";
            std::string msg = std::string(line.tag().data()) + ":" + threadId + ": " + line.message().data();
            if (rtcLogCallback)
            {
              rtcLogCallback((int)line.severity(), line.filename().data(), line.line(), msg.c_str());
            }            
        }
    };

    rtc::LogSink *libwebrtcRedirectLogSink()
    {
        static WebRTCRedirectLogSink logSink;
        return &logSink;
    }

// Initialize static variable g_is_initialized to false.
static bool g_is_initialized = false;

// Initializes SSL, if not initialized.
bool LibWebRTC::Initialize() {
  if (!g_is_initialized) {
    rtc::InitializeSSL();
    rtc::LogMessage::LogThreads(true);
    rtc::LogMessage::LogToDebug(rtc::LoggingSeverity::LS_NONE);
    rtc::LogMessage::AddLogToStream(libwebrtcRedirectLogSink(), rtc::LoggingSeverity::LS_VERBOSE);
    g_is_initialized = true;
  }
  return g_is_initialized;
}

// Stops and cleans up the threads and SSL.
void LibWebRTC::Terminate() {
  rtc::ThreadManager::Instance()->SetCurrentThread(NULL);
  rtc::CleanupSSL();

  // Resets the static variable g_is_initialized to false.
  g_is_initialized = false;
}

// Creates and returns an instance of RTCPeerConnectionFactory.
scoped_refptr<RTCPeerConnectionFactory>
LibWebRTC::CreateRTCPeerConnectionFactory() {
  scoped_refptr<RTCPeerConnectionFactory> rtc_peerconnection_factory =
      scoped_refptr<RTCPeerConnectionFactory>(
          new RefCountedObject<RTCPeerConnectionFactoryImpl>());
  rtc_peerconnection_factory->Initialize();
  return rtc_peerconnection_factory;
}

void LibWebRTC::RegisterLogCallback(LogCallback callback)
{
  rtcLogCallback = callback;
}

}  // namespace libwebrtc
