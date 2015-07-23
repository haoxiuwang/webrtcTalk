
#ifndef TALK_MEDIA_DEVICES_APPRTCVIDEORENDERER_H_
#define TALK_MEDIA_DEVICES_APPRTCVIDEORENDERER_H_

#include "talk/media/base/videorenderer.h"
#include "talk/media/base/videoframe.h"

namespace cricket {

class ApprtcVideoRenderer : public VideoRenderer {
 public:
  ApprtcVideoRenderer(int x, int y, bool isLocal);
  virtual ~ApprtcVideoRenderer();

  virtual bool SetSize(int width, int height, int reserved);
  virtual bool RenderFrame(const VideoFrame* frame);

 private:
  bool first = true;
  bool isLocal_;
 };

}
#endif
