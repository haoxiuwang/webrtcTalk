
#ifndef TALK_MEDIA_DEVICES_MACCONSOLEVIDEORENDERER_H_
#define TALK_MEDIA_DEVICES_MACCONSOLEVIDEORENDERER_H_


#include "talk/media/base/videorenderer.h"
#include "talk/media/devices/filevideocapturer.h"
#include "webrtc/video_frame.h"
#include "webrtc/modules/video_render/include/video_render.h"
#include "webrtc/modules/video_render/include/video_render_defines.h"

class CocoaRenderViewCreator;

namespace cricket {

class MacConsoleVideoRenderer : public VideoRenderer {
 public:
  MacConsoleVideoRenderer(int x, int y, bool isLocal);
  virtual ~MacConsoleVideoRenderer();

  virtual bool SetSize(int width, int height, int reserved);
  virtual bool RenderFrame(const VideoFrame* frame);
  void ConvertToI420VideoFrame(const VideoFrame* frame);

 private:
  webrtc::CocoaRenderViewCreator* CocoaRVC_ = NULL;
  webrtc::I420VideoFrame* frame_ = NULL;
  void* window_ = NULL;
  webrtc::VideoRender* renderModule_ = NULL;
  webrtc::VideoRenderCallback* renderCallback0 = NULL;
  const int streamId0 = 0;
  int error = 0;
};

}

#endif
