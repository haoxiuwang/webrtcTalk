/*
 * libjingle
 * Copyright 2004 Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// Definition of class GdiVideoRenderer that implements the abstract class
// cricket::VideoRenderer via GDI on Windows.

#ifndef TALK_MEDIA_DEVICES_GDIVIDEORENDERER_H_
#define TALK_MEDIA_DEVICES_GDIVIDEORENDERER_H_

#ifdef WIN32
#include "talk/media/base/videorenderer.h"
#include "webrtc/base/scoped_ptr.h"
#include "webrtc/base/thread.h"
#include "webrtc/base/win32window.h"

namespace cricket {

class GdiVideoRenderer : public VideoRenderer {
	public:
		GdiVideoRenderer(int x, int y);
		virtual ~GdiVideoRenderer();

		// Implementation of pure virtual methods of VideoRenderer.
		// These two methods may be executed in different threads.
		// SetSize is called before RenderFrame.
		virtual bool SetSize(int width, int height, int reserved);
		virtual bool RenderFrame(const VideoFrame* frame);

	private:
		rtc::scoped_ptr<VideoWindow> window_;
		// The initial position of the window.
		int initial_x_;
		int initial_y_;
};

//edited
class VideoWindow : public rtc::Win32Window {
	public:
		VideoWindow(int x, int y, int width, int height, bool local);
		virtual ~VideoWindow();

		// Called when the video size changes. If it is called the first time, we
		// create and start the thread. Otherwise, we send kSetSizeMsg to the thread.
		// Context: non-worker thread.
		bool SetSize(int width, int height);

		// Called when a new frame is available. Upon this call, we send
		// kRenderFrameMsg to the window thread. Context: non-worker thread. It may be
		// better to pass RGB bytes to VideoWindow. However, we pass VideoFrame to put
		// all the thread synchronization within VideoWindow.
		bool RenderFrame(const VideoFrame* frame);

	protected:
		// Override virtual method of rtc::Win32Window. Context: worker Thread.
		virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam,
			LRESULT& result);

	private:
		enum { kSetSizeMsg = WM_USER, kRenderFrameMsg};

		class WindowThread : public rtc::Thread {
		public:
			explicit WindowThread(VideoWindow* window) : window_(window) {}

			virtual ~WindowThread() {
				Stop();
			}

			// Override virtual method of rtc::Thread. Context: worker Thread.
			virtual void Run() {
				// Initialize the window
				if (!window_ || !window_->Initialize()) {
					return;
				}
				// Run the message loop
				MSG msg;
				while (GetMessage(&msg, NULL, 0, 0) > 0) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}

		private:
			VideoWindow* window_;
		};

		// Context: worker Thread.
		bool Initialize();
		void OnPaint();
		void OnSize(int width, int height, bool frame_changed);
		void OnRenderFrame(const VideoFrame* frame);

		BITMAPINFO bmi_;
		rtc::scoped_ptr<uint8[]> image_;
		rtc::scoped_ptr<WindowThread> window_thread_;
		// The initial position of the window.
		int initial_x_;
		int initial_y_;
		int initial_width_;
		int initial_height_;
		bool _local;
};

class BabelVideoRenderer : public VideoRenderer {
public:
	BabelVideoRenderer(int x, int y,bool isRemote);
	virtual ~BabelVideoRenderer();

	// Implementation of pure virtual methods of VideoRenderer.
	// These two methods may be executed in different threads.
	// SetSize is called before RenderFrame.
	virtual bool SetSize(int width, int height, int reserved);
	virtual bool RenderFrame(const VideoFrame* frame);

private:
	//class VideoWindow;  // forward declaration, defined in the .cc file
	//rtc::scoped_ptr<VideoWindow> window_;
	//// The initial position of the window.
	int initial_x_;
	int initial_y_;
	bool _local;
	BITMAPINFO bmi_;
	rtc::scoped_ptr<uint8[]> image_;

};

class Win32VideoRenderer : public VideoRenderer {
public:
	Win32VideoRenderer(int x, int y,bool isRemote);
	virtual ~Win32VideoRenderer();

	// Implementation of pure virtual methods of VideoRenderer.
	// These two methods may be executed in different threads.
	// SetSize is called before RenderFrame.
	virtual bool SetSize(int width, int height, int reserved);
	virtual bool RenderFrame(const VideoFrame* frame);

private:
	//class VideoWindow;  // forward declaration, defined in the .cc file
	//rtc::scoped_ptr<VideoWindow> window_;
	//// The initial position of the window.
	bool _local;
	BITMAPINFO bmi_;
	rtc::scoped_ptr<uint8[]> image_;

};

}  // namespace cricket

#endif  // WIN32
#endif  // TALK_MEDIA_DEVICES_GDIVIDEORENDERER_H_
