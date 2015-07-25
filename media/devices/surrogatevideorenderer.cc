#include "webrtc/base/logging.h"
#include "talk/media/base/videoframe.h"
#include "talk/media/devices/surrogatevideorenderer.h"
//WH
#include "webrtc/libjingle/examples/call/PublicCallback.h"
//#include "webrtc/modules/utility/interface/helpers_android.h"

using namespace webrtc;

namespace cricket {

SurrogateVideoRenderer::SurrogateVideoRenderer(int x, int y, void* window, webrtc::VideoRenderType windowType)
{
  frame_ = new I420VideoFrame();
  windowType_ = windowType;

  int myId = 12345;
  renderModule_ = VideoRender::CreateVideoRender(myId,
						 window,
                                                 false,
                                                 windowType);
}

SurrogateVideoRenderer::~SurrogateVideoRenderer() {

 /* WH 待删 if(window_){
    AttachThreadScoped ats(g_vm);
    ats.env()->DeleteGlobalRef(reinterpret_cast<jobject>(window_));
    window_ = NULL;
 }*/
    error = renderModule_->StopRender(streamId0);
    assert(error == 0);

    error = renderModule_->DeleteIncomingRenderStream(streamId0);
    assert(error == 0);
    delete frame_;
    frame_ = NULL;

}

bool SurrogateVideoRenderer::SetSize(int width, int height, int reserved) {

if(!renderCallback0){
  renderCallback0 = renderModule_->AddIncomingRenderStream(streamId0, 0, 0.0f, 0.0f, 1.0f, 1.0f);

  error = renderModule_->StartRender(streamId0);
  if (error != 0) {
        assert(false);
    }
}


return true;

}

bool SurrogateVideoRenderer::RenderFrame(const VideoFrame* frame) {

callBack_Test_Method("SurrogateVideoRenderer::RenderFrame");

ConvertToI420VideoFrame(frame);

renderCallback0->RenderFrame(streamId0, *frame_);

return true;
}

void SurrogateVideoRenderer::ConvertToI420VideoFrame(const VideoFrame* frame){

int size_y = frame->GetYPitch() * frame->GetHeight();
int size_u = frame->GetUPitch() * frame->GetHeight();
int size_v = frame->GetVPitch() * frame->GetHeight();

frame_->CreateFrame(size_y, frame->GetYPlane(), size_u, frame->GetUPlane(), size_v, frame->GetVPlane(), frame->GetWidth(), frame->GetHeight(), frame->GetYPitch(), frame->GetUPitch(), frame->GetVPitch());

return;
}

}  // namespace cricket
