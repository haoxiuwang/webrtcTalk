#ifndef TALK_MEDIA_DEVICES_IOSVIDEORENDERER_H_
#define TALK_MEDIA_DEVICES_IOSVIDEORENDERER_H_

#include "talk/media/base/videorenderer.h"
#include "webrtc/video_frame.h"
#include "webrtc/modules/video_render/include/video_render.h"
#include "webrtc/modules/video_render/include/video_render_defines.h"

namespace cricket {


class IOSVideoRenderer : public VideoRenderer {
 public:
  IOSVideoRenderer(int x, int y, bool isLocal);
  virtual ~IOSVideoRenderer();

  virtual bool SetSize(int width, int height, int reserved);
  virtual bool RenderFrame(const VideoFrame* frame);
  void ConvertToI420VideoFrame(const VideoFrame* frame);

 private:
  webrtc::I420VideoFrame* frame_ = NULL;
  webrtc::VideoRenderType windowType_;
  webrtc::VideoRender* renderModule_ = NULL;
  webrtc::VideoRenderCallback* renderCallback0 = NULL;
  const int streamId0 = 0;
  int error = 0;

  bool first = true;

  bool isLocal_;
};

}

#endif
