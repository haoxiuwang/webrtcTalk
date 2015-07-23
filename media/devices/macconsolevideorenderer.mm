#include "talk/media/base/videoframe.h"
#include "webrtc/base/logging.h"
#include "talk/media/devices/macconsolevideorenderer.h"
#include "webrtc/modules/video_render/test/testAPI/CocoaRenderViewCreator.h"

using namespace webrtc;

namespace cricket {

MacConsoleVideoRenderer::MacConsoleVideoRenderer(int x, int y, bool isLocal)
{
  CocoaRVC_ = new CocoaRenderViewCreator();
  frame_ = new I420VideoFrame();
}

MacConsoleVideoRenderer::~MacConsoleVideoRenderer() {

    error = renderModule_->StopRender(streamId0);
    assert(error == 0);

    error = renderModule_->DeleteIncomingRenderStream(streamId0);
    assert(error == 0);
    delete frame_;
    frame_ = NULL;

}

bool MacConsoleVideoRenderer::SetSize(int width, int height, int reserved) {
if(window_ == NULL){
   window_ = CocoaRVC_->init(width, height);

int myId = 12345;
VideoRenderType windowType = kRenderCocoa;
renderModule_ = VideoRender::CreateVideoRender(myId,
                                                  window_,
                                                  false,
                                                  windowType);

renderCallback0 = renderModule_->AddIncomingRenderStream(streamId0, 0, 0.0f, 0.0f, 1.0f, 1.0f);

LOG(LS_WARNING)<<"Start render";
    error = renderModule_->StartRender(streamId0);
    if (error != 0) {
        assert(false);
    }
}

return true;

}

bool MacConsoleVideoRenderer::RenderFrame(const VideoFrame* frame) {
LOG(LS_WARNING)<<" MacConsoleVideoRenderer::RenderFrame ";

ConvertToI420VideoFrame(frame);

renderCallback0->RenderFrame(streamId0, *frame_);

return true;
}

void MacConsoleVideoRenderer::ConvertToI420VideoFrame(const VideoFrame* frame){

int size_y = frame->GetYPitch() * frame->GetHeight();
int size_u = frame->GetUPitch() * frame->GetHeight();
int size_v = frame->GetVPitch() * frame->GetHeight();

frame_->CreateFrame(size_y, frame->GetYPlane(), size_u, frame->GetUPlane(), size_v, frame->GetVPlane(), frame->GetWidth(), frame->GetHeight(), frame->GetYPitch(), frame->GetUPitch(), frame->GetVPitch());

return;
}

}  // namespace cricket
