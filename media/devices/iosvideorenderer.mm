#import "webrtc/modules/video_render/ios/video_render_ios_view.h"
#include "webrtc/base/logging.h"
#include "talk/media/base/videoframe.h"
#include "talk/media/devices/iosvideorenderer.h"
#include "webrtc/libjingle/examples/call/PublicCallback.h"

using namespace webrtc;

namespace cricket {

VideoRenderIosView* IOSLocalView_ = NULL;
VideoRenderIosView* IOSRemoteView_ = NULL;

IOSVideoRenderer::IOSVideoRenderer(int x, int y, bool isLocal)
{
  isLocal_ = isLocal;
  frame_ = new I420VideoFrame();
  //windowType_ = windowType;

  int myId = 12345;
  
  void* lv_ = reinterpret_cast<void*>(IOSLocalView_);
  void* rv_ = reinterpret_cast<void*>(IOSRemoteView_);

  if(isLocal && IOSLocalView_ != NULL)
     renderModule_ = VideoRender::CreateVideoRender(myId,
						 lv_,
                                                 false,
                                                 webrtc::VideoRenderType::kRenderiOS);
  else if(isLocal == false && IOSRemoteView_ != NULL)
     renderModule_ = VideoRender::CreateVideoRender(myId,
						 rv_,
                                                 false,
                                                 webrtc::VideoRenderType::kRenderiOS);
}

IOSVideoRenderer::~IOSVideoRenderer() {
   //if(renderModule_!= NULL) {
    
    error = renderModule_->StopRender(streamId0);
    assert(error == 0);

    error = renderModule_->DeleteIncomingRenderStream(streamId0);
    assert(error == 0);
    delete frame_;
    frame_ = NULL;
//}
}

bool IOSVideoRenderer::SetSize(int width, int height, int reserved) {
 //if(renderModule_!= NULL) {
if(!renderCallback0){
  renderCallback0 = renderModule_->AddIncomingRenderStream(streamId0, 0, 0.0f, 0.0f, 1.0f, 1.0f);

  error = renderModule_->StartRender(streamId0);
  if (error != 0) {
        assert(false);
    }
}//}


return true;

}

bool IOSVideoRenderer::RenderFrame(const VideoFrame* frame) {
 //if(renderModule_!= NULL) {
ConvertToI420VideoFrame(frame);

renderCallback0->RenderFrame(streamId0, *frame_);

return true;
}

void IOSVideoRenderer::ConvertToI420VideoFrame(const VideoFrame* frame){

int size_y = frame->GetYPitch() * frame->GetHeight();
int size_u = frame->GetUPitch() * frame->GetHeight();
int size_v = frame->GetVPitch() * frame->GetHeight();

frame_->CreateFrame(size_y, frame->GetYPlane(), size_u, frame->GetUPlane(), size_v, frame->GetVPlane(), frame->GetWidth(), frame->GetHeight(), frame->GetYPitch(), frame->GetUPitch(), frame->GetVPitch());

/*
if(first){
char buf[10];

sprintf(buf, "%d", frame->GetWidth());
std::string w = buf;	

sprintf(buf, "%d", frame->GetHeight());
std::string h = buf;	

sprintf(buf, "%d", frame-> GetYPitch());
std::string yp = buf;

sprintf(buf, "%d", frame-> GetUPitch());
std::string up = buf;

sprintf(buf, "%d", frame-> GetVPitch());
std::string vp = buf;


MyPrint("FrameWidth = " + w + ", FrameHeight = " + h + ", YPitch = " + yp + ", UPitch = " + up + ", VPitch = " + vp);

}

first = false;*/

return;
}

}  // namespace cricket
