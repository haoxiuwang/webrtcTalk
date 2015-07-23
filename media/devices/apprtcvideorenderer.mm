
#import "talk/app/webrtc/objc/public/RTCI420Frame.h"
#include "talk/media/devices/apprtcvideorenderer.h"
#include "webrtc/libjingle/examples/call/PublicCallback.h"
#import "webrtc/libjingle/examples/call/ARDP2PCallback.h"

using namespace webrtc;

namespace cricket {

ApprtcVideoRenderer::ApprtcVideoRenderer(int x, int y, bool isLocal)
{
  isLocal_ = isLocal;
}

ApprtcVideoRenderer::~ApprtcVideoRenderer() {
}

bool ApprtcVideoRenderer::SetSize(int width, int height, int reserved) {
	ARDP2PCallback* ocCallback = (__bridge ARDP2PCallback*) ocArdCallback;
	[ocCallback setSizeX:(CGFloat)width sizeY:(CGFloat)height isLocal:isLocal_];
	return true;
}
int num = 0;
bool ApprtcVideoRenderer::RenderFrame(const VideoFrame* frame) {
LOG(LS_WARNING) << "ApprtcVideoRenderer::RenderFrame";
if(num++ < 10){
LOG(LS_WARNING) << "ApprtcVideoRenderer::RenderFrame2";

/*char buf[10];
sprintf(buf, "%d", frame->GetWidth());
std::string w = buf;	

sprintf(buf, "%d", frame->GetHeight());
std::string h = buf;	

sprintf(buf, "%d", frame-> GetYPitch());
std::string yp = buf;

sprintf(buf, "%d", frame-> GetUPitch());
std::string up = buf;

sprintf(buf, "%d", frame-> GetVPitch());
std::string vp = buf;*/


LOG(LS_WARNING) << "FrameWidth = " << frame->GetWidth() << ", YPitch = " << frame-> GetYPitch() << ", ChromaWidth = " << frame->GetChromaWidth() << ", UPitch = " << frame-> GetUPitch();

}
/*
first = false;*/



  RTCI420Frame* i420Frame = [[RTCI420Frame alloc] initWithVideoFrame:frame];
  ARDP2PCallback* ocCallback = (__bridge ARDP2PCallback*) ocArdCallback;
  [ocCallback renderFrame:i420Frame isLocal:isLocal_];
  return true;
}

} // namespace cricket
