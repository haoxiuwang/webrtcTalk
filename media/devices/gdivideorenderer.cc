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

// Implementation of GdiVideoRenderer on Windows

#ifdef WIN32

#include "talk/media/devices/gdivideorenderer.h"

#include "talk/media/base/videocommon.h"
#include "talk/media/base/videoframe.h"

namespace cricket {

	VideoWindow::VideoWindow(
		int x, int y, int width, int height, bool local)
		: initial_x_(x),
		initial_y_(y),
		initial_width_(width),
		initial_height_(height),
		_local(local){
			memset(&bmi_.bmiHeader, 0, sizeof(bmi_.bmiHeader));
			bmi_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi_.bmiHeader.biPlanes = 1;
			bmi_.bmiHeader.biBitCount = 32;
			bmi_.bmiHeader.biCompression = BI_RGB;
			bmi_.bmiHeader.biWidth = width;
			bmi_.bmiHeader.biHeight = -height;
			bmi_.bmiHeader.biSizeImage = width * height * 4;

			image_.reset(new uint8[bmi_.bmiHeader.biSizeImage]);

			//std:: stringstream ss;
			//ss<<bmi_.bmiHeader.biWidth;
			//ManagedDataWriter writer(2);
			//writer.WriteString("video VideoWindow()");
			//writer.WriteString(ss.str().c_str());
			//NewDayCallback(R_DEBUG_INFO, writer.ToArray());
	}

	VideoWindow::~VideoWindow() {
		// Context: caller Thread. We cannot call Destroy() since the window was
		// created by another thread. Instead, we send WM_CLOSE message.
		if (handle()) {
			SendMessage(handle(), WM_CLOSE, 0, 0);
		}
	}

	bool VideoWindow::SetSize(int width, int height) {

		std:: stringstream ss;

		if (!window_thread_.get()) {
			// Create and start the window thread.
			ss<<"windowthread()   ";
			window_thread_.reset(new WindowThread(this));
			return window_thread_->Start();

		} else if (width != bmi_.bmiHeader.biWidth ||
			height != -bmi_.bmiHeader.biHeight) {
				ss<<"kSetSizeMsg()   ";
				SendMessage(handle(), kSetSizeMsg, 0, MAKELPARAM(width, height));
		}

		//ss<<bmi_.bmiHeader.biWidth;
		//ManagedDataWriter writer(2);
		//writer.WriteString("video SetSize()");
		//writer.WriteString(ss.str().c_str());
		//NewDayCallback(R_DEBUG_INFO, writer.ToArray());

		return true;
	}

	bool VideoWindow::RenderFrame(const VideoFrame* frame) {
		if (!handle()) {
			return false;
		}

		SendMessage(handle(), kRenderFrameMsg, reinterpret_cast<WPARAM>(frame), 0);
		return true;
	}

	bool VideoWindow::OnMessage(UINT uMsg, WPARAM wParam,
		LPARAM lParam, LRESULT& result) {
			switch (uMsg) {
			case WM_PAINT:
				OnPaint();
				return true;

			case WM_DESTROY:
				PostQuitMessage(0);  // post WM_QUIT to end the message loop in Run()
				return false;

			case WM_SIZE:  // The window UI was resized.
				OnSize(LOWORD(lParam), HIWORD(lParam), false);
				return true;

			case kSetSizeMsg:  // The video resolution changed.
				OnSize(LOWORD(lParam), HIWORD(lParam), true);
				return true;

			case kRenderFrameMsg:
				OnRenderFrame(reinterpret_cast<const VideoFrame*>(wParam));
				return true;
			}
			return false;
	}

	bool VideoWindow::Initialize() {

		/*if (!talk_base::Win32Window::Create(
		NULL, L"Video Renderer",
		WS_OVERLAPPEDWINDOW | WS_SIZEBOX,
		WS_EX_APPWINDOW,
		initial_x_, initial_y_,

		bmi_.bmiHeader.biWidth, -bmi_.bmiHeader.biHeight)) {
		return false;
		}*/

		//	RECT desktop;
		//const HWND hDesktop = GetDesktopWindow();
		//GetWindowRect(hDesktop, &desktop);

		if (!talk_base::Win32Window::Create(
			NULL, L"",
			WS_VISIBLE,
			0,
			0, 0, //desktop.right, desktop.bottom))
			initial_x_, initial_y_)) 
		{
			return false;
		}

		/*std:: stringstream ss;
		ss<<bmi_.bmiHeader.biWidth;
		ss<<"   ";
		ss<<bmi_.bmiHeader.biHeight;

		ManagedDataWriter writer(2);
		writer.WriteString("video Initialize()");
		writer.WriteString(ss.str().c_str());
		NewDayCallback(R_DEBUG_INFO, writer.ToArray());*/

		ManagedDataWriter writer1(1);
		writer1.WritePointer((void*)handle());
		NewDayCallback(_local ? R_LOCAL_RENDERER_HANDLE : R_REMOTE_RENDERER_HANDLE, writer1.ToArray());

		// Get window and client sizes
		RECT rcClient, rcWindow;
		GetClientRect(handle(), &rcClient);
		GetWindowRect(handle(), &rcWindow);

		// Find offset between window size and client size
		POINT ptDiff;
		ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
		ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;

		// Resize client
		MoveWindow(handle(), rcWindow.left, rcWindow.top,
			initial_width_, initial_height_, false);
		UpdateWindow(handle());
		ShowWindow(handle(), SW_SHOW);


		//std:: stringstream ss;
		//ss<<initial_width_ + ptDiff.x;
		//ss<<"   ";
		//ss<<rcClient.right;
		//ss<<"   ";
		//ss<<bmi_.bmiHeader.biWidth;

		//ManagedDataWriter writer(2);
		//writer.WriteString("video Initialize()");
		//writer.WriteString(ss.str().c_str());
		//NewDayCallback(R_DEBUG_INFO, writer.ToArray());

		OnSize(bmi_.bmiHeader.biWidth, -bmi_.bmiHeader.biHeight, false);
		return true;
	}

	int i = 1;

	void VideoWindow::OnPaint() {
		RECT rcClient;
		GetClientRect(handle(), &rcClient);

		rcClient.left=0;
		//rcClient.right=bmi_.bmiHeader.biWidth;
		rcClient.top=0;
		//rcClient.bottom=-bmi_.bmiHeader.biHeight;

		/* LONG dx = 0;
		LONG dy = 0; 
		LONG dw = 0; 
		LONG dh = 0;
		double sratio = (double)bmi_.bmiHeader.biWidth / -bmi_.bmiHeader.biHeight;
		double wratio = (double)rcClient.right / rcClient.bottom;

		if(sratio > wratio ) {
		dw = rcClient.right;
		dh = dw / sratio ;
		dy = (rcClient.bottom - dh) / 2;
		}
		else if(sratio < wratio ) {
		dh = rcClient.bottom;
		dw = dh * sratio ;
		dx = (rcClient.right - dw) / 2;
		}
		else {
		dw = rcClient.right;
		dh = rcClient.bottom;
		}


		//ss<<"   ";
		//ss<<dx;
		//ss<<"   ";
		//ss<<dw;
		//ss<<"   ";
		//ss<<dh;
		*/
		if(++i < 2 || i == 10){
			//std:: stringstream ss;

			// ss<<rcClient.right;
			// ss<<"   ";
			// ss<<bmi_.bmiHeader.biWidth;

			// ManagedDataWriter writer(2);
			// writer.WriteString("video OnPaint()");
			// writer.WriteString(ss.str().c_str());
			// NewDayCallback(R_DEBUG_INFO, writer.ToArray());

		}

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(handle(), &ps);

		if(rcClient.right < bmi_.bmiHeader.biWidth || rcClient.bottom < -bmi_.bmiHeader.biHeight)
			SetStretchBltMode(hdc, COLORONCOLOR); 

		StretchDIBits(hdc,
			0, 0, rcClient.right,rcClient.bottom,
			//dx, dy, dw, dh,  // destination rect
			0, 0, bmi_.bmiHeader.biWidth, -bmi_.bmiHeader.biHeight,  // source rect
			image_.get(), &bmi_, DIB_RGB_COLORS, SRCCOPY);
		EndPaint(handle(), &ps);
	}

	void VideoWindow::OnSize(int width, int height,
		bool frame_changed) {
			//// Get window and client sizes
			//RECT rcClient, rcWindow;
			//GetClientRect(handle(), &rcClient);
			//GetWindowRect(handle(), &rcWindow);

			//// Find offset between window size and client size
			//POINT ptDiff;
			//ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
			//ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;

			//// Resize client
			//MoveWindow(handle(), rcWindow.left, rcWindow.top,
			//           width + ptDiff.x, height + ptDiff.y, false);
			//UpdateWindow(handle());
			//ShowWindow(handle(), SW_SHOW);

			if (frame_changed && (width != bmi_.bmiHeader.biWidth ||
				height != -bmi_.bmiHeader.biHeight)) {

					//std:: stringstream ss;
					//ss<<bmi_.bmiHeader.biWidth;
					//ss<< "  ";
					//ss<<width;

					//ManagedDataWriter writer(2);
					//writer.WriteString("video OnSize()");
					//writer.WriteString(ss.str().c_str());
					//NewDayCallback(R_DEBUG_INFO, writer.ToArray());

					// Update the bmi and image buffer
					bmi_.bmiHeader.biWidth = width;
					bmi_.bmiHeader.biHeight = -height;
					bmi_.bmiHeader.biSizeImage = width * height * 4;
					image_.reset(new uint8[bmi_.bmiHeader.biSizeImage]);
			}
	}

	int ir = 0;
	void VideoWindow::OnRenderFrame(const VideoFrame* frame) {
		if (!frame) {
			return;
		}
		if(ir == 0){
			std:: stringstream ss;
			ss<<bmi_.bmiHeader.biWidth;

			/* ManagedDataWriter writer(2);
			writer.WriteString("video OnRenderFrame()");
			writer.WriteString(ss.str().c_str());
			NewDayCallback(R_DEBUG_INFO, writer.ToArray());*/
			ir++;
		}
		// Convert frame to ARGB format, which is accepted by GDI
		frame->ConvertToRgbBuffer(cricket::FOURCC_ARGB, image_.get(),
			bmi_.bmiHeader.biSizeImage,
			bmi_.bmiHeader.biWidth * 4);
		InvalidateRect(handle(), 0, 0);
	}


	/////////////////////////////////////////////////////////////////////////////
	// Implementation of class GdiVideoRenderer
	/////////////////////////////////////////////////////////////////////////////
	GdiVideoRenderer::GdiVideoRenderer(int x, int y)
		: initial_x_(x),
		initial_y_(y) {
	}
	GdiVideoRenderer::~GdiVideoRenderer() {}

	bool GdiVideoRenderer::SetSize(int width, int height, int reserved) {
		if (!window_.get()) {  // Create the window for the first frame
			window_.reset(new VideoWindow(initial_x_, initial_y_, width, height));
		}
		return window_->SetSize(width, height);
	}

	bool GdiVideoRenderer::RenderFrame(const VideoFrame* frame) {
		if (!frame || !window_.get()) {
			return false;
		}
		return window_->RenderFrame(frame);
	}

	
	/////////////////////////////////////////////////////////////////////////////
	// Implementation of class BabelVideoRenderer 每帧均为i420格式
	/////////////////////////////////////////////////////////////////////////////

	BabelVideoRenderer::BabelVideoRenderer(int x, int y,bool local)
		: initial_x_(x),
		initial_y_(y),
		_local(local){
			memset(&bmi_.bmiHeader, 0, sizeof(bmi_.bmiHeader));
			bmi_.bmiHeader.biCompression=BI_RGB ;
			bmi_.bmiHeader.biBitCount=32;
			bmi_.bmiHeader.biPlanes=1;
			bmi_.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	}

	BabelVideoRenderer::~BabelVideoRenderer() {}

	bool BabelVideoRenderer::SetSize(int width, int height, int reserved) 
	{
		if (width != bmi_.bmiHeader.biWidth ||
			height != -bmi_.bmiHeader.biHeight) {
				// Update the bmi and image buffer
				bmi_.bmiHeader.biWidth = width;		
				bmi_.bmiHeader.biHeight = -height;			
				bmi_.bmiHeader.biSizeImage = width * height * 4;
				image_.reset(new uint8[bmi_.bmiHeader.biSizeImage]);

				ManagedDataWriter writer(3);
				writer.WritePointer((void*)image_.get());
				writer.WritePointer((void*)width);
				writer.WritePointer((void*)height);

				if(_local)
					NewDayCallback(R_LOCAL_FRAME_METADATA,writer.ToArray());
				else
					NewDayCallback(R_REMOTE_FRAME_METADATA,writer.ToArray());
		}
		return true;
	}

	bool BabelVideoRenderer::RenderFrame(const VideoFrame* frame) 
	{
		//size_t size_ = frame->SizeOf(frame->GetWidth(), frame->GetHeight());
		//uint8* buffer_ = new uint8[size_];
		size_t actual_size_ = frame->CopyToBuffer(image_.get(), bmi_.bmiHeader.biSizeImage);


		if(_local)
		{
			NewDayCallback(R_LOCAL_FRAME_COMING, NULL);
		}
		else
		{
			NewDayCallback(R_REMOTE_FRAME_COMING, NULL);
		}

		return true;	
	}

	/////////////////////////////////////////////////////////////////////////////
	// Implementation of class Win32VideoRenderer 每帧均为ARGB格式
	/////////////////////////////////////////////////////////////////////////////

	Win32VideoRenderer::Win32VideoRenderer(int x, int y,bool local)
		: _local(local){
			memset(&bmi_.bmiHeader, 0, sizeof(bmi_.bmiHeader));
			bmi_.bmiHeader.biCompression=BI_RGB ;
			bmi_.bmiHeader.biBitCount=32;
			bmi_.bmiHeader.biPlanes=1;
			bmi_.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	}

	Win32VideoRenderer::~Win32VideoRenderer() {}

	bool Win32VideoRenderer::SetSize(int width, int height, int reserved) 
	{
		//if (width != bmi_.bmiHeader.biWidth ||
		//	height != -bmi_.bmiHeader.biHeight) {
		// Update the bmi and image buffer
		bmi_.bmiHeader.biWidth = width;		
		bmi_.bmiHeader.biHeight = -height;			
		bmi_.bmiHeader.biSizeImage = width * height * 4;
		image_.reset(new uint8[bmi_.bmiHeader.biSizeImage]);

		ManagedDataWriter writer(3);
		writer.WritePointer((void*)image_.get());
		writer.WritePointer((void*)width);
		writer.WritePointer((void*)height);

		if(_local)
			NewDayCallback(R_LOCAL_FRAME_METADATA,writer.ToArray());
		else
			NewDayCallback(R_REMOTE_FRAME_METADATA,writer.ToArray());
		//}
		return true;
	}

	bool Win32VideoRenderer::RenderFrame(const VideoFrame* frame) 
	{
		//size_t size_ = frame->SizeOf(frame->GetWidth(), frame->GetHeight());
		//uint8* buffer_ = new uint8[size_];
		//size_t actual_size_ = frame->CopyToBuffer(image_.get(), bmi_.bmiHeader.biSizeImage);

		frame->ConvertToRgbBuffer(cricket::FOURCC_ARGB, image_.get(),
			bmi_.bmiHeader.biSizeImage,
			bmi_.bmiHeader.biWidth * 4);

		if(_local)
		{
			NewDayCallback(R_LOCAL_FRAME_COMING, NULL);
		}
		else
		{
			NewDayCallback(R_REMOTE_FRAME_COMING, NULL);
		}

		return true;	
	}

}  // namespace cricket
#endif  // WIN32
