/*
* Copyright 2006-2008 Appcelerator, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "ti_web_shell.h"
#include "ti_user_window.h"
#include "ti_utils.h"
#include "ti_menu.h"
#include "Resource.h"

#include <fstream>
#include <shellapi.h>

TCHAR TiWebShell::defaultWindowTitle[128];
TCHAR TiWebShell::windowClassName[128];
TiAppConfig* TiWebShell::tiAppConfig = NULL;
std::vector<TiWebShell*> TiWebShell::openShells = std::vector<TiWebShell*>();

/*static*/
TiWebShell* TiWebShell::fromWindow(HWND hWnd) {
  return reinterpret_cast<TiWebShell*>(win_util::GetWindowUserData(hWnd));
}

/*static*/
TiWebShell* TiWebShell::getMainTiWebShell() {
	if (openShells.size() > 0) {
		return openShells[0];
	}

	return NULL;
}

/*static*/
void TiWebShell::initWindowClass ()
{
	HINSTANCE hInstance = ::GetModuleHandle(NULL);
	LoadString(hInstance, IDS_APP_TITLE, defaultWindowTitle, 128);
	LoadString(hInstance, IDC_CHROME_SHELL3, windowClassName, 128);
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= TiWebShell::WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHROME_SHELL3));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= windowClassName;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	
	RegisterClassEx(&wcex);
}

/*static*/
LRESULT CALLBACK TiWebShell::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId, wmEvent;

	TiWebShell *tiWebShell = TiWebShell::fromWindow(hWnd);

	switch (message)
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
					//DialogBox(tiWebShell->getInstance(), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
				{
					LRESULT handled = TiMenu::handleMenuClick(hWnd, message, wParam, lParam);

					if(! handled)
					{
						return DefWindowProc(hWnd, message, wParam, lParam);
					}
				}
			}
			break;

		// forward some things onto the webview host
		case WM_MOUSEMOVE:
		case WM_MOUSELEAVE:
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_CAPTURECHANGED:
		case WM_CANCELMODE:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_IME_CHAR:
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
		case WM_MOUSEWHEEL:
			if (!PostMessage(tiWebShell->getHost()->window_handle(), message, wParam, lParam)) {
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_DESTROY:
			if (tiWebShell == TiWebShell::getMainTiWebShell()) {
				PostQuitMessage(0);
			}
			break;
		case WM_SIZE:
			tiWebShell->resizeHost();
			break;
		case TI_TRAY_CLICKED:
			{
				UINT uMouseMsg = (UINT) lParam;
				if(uMouseMsg == WM_LBUTTONDOWN)
				{
					// handle the click callback for the tray
				}
				else if (uMouseMsg == WM_RBUTTONDOWN)
				{
					TiMenu::showSystemMenu();
				}
			}
			break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

TiWebShell::TiWebShell(TiWindow *window) : hWnd(NULL), hInstance(NULL), host(NULL)
{
	TiWebShell::openShells.push_back(this);	
	this->tiWindow = window;

	if (window != NULL) {
		this->url = tiWindow->getURL();
	}

	tiUserWindow = new TiUserWindow(this);
}

TiWebShell::TiWebShell(const char *url) : hWnd(NULL), hInstance(NULL), host(NULL)
{
	TiWebShell::openShells.push_back(this);	
	this->url = url;
	this->tiWindow = new TiWindow();
	
	tiUserWindow = new TiUserWindow(this);
}

TiWebShell::~TiWebShell(void) {
	// TODO  remove tray icon if one exists .. mmm.. 
	//this->removeTrayIcon();
}

void TiWebShell::createWindow()
{
	hInstance = ::GetModuleHandle(NULL);
	hWnd = CreateWindowEx(WS_EX_LAYERED, windowClassName, defaultWindowTitle,
                           WS_CLIPCHILDREN,
                           0, 0, 0, 0,
                           NULL, NULL, hInstance, NULL);

	win_util::SetWindowUserData(hWnd, this);
	
	static WebPreferences webPrefs = ti_initWebPrefs();
	webViewDelegate = new TiWebViewDelegate(this);
	host = WebViewHost::Create(hWnd, webViewDelegate, webPrefs);
	webViewDelegate->setWebViewHost(host);
	
	reloadTiWindow();
}

#define SetFlag(x,flag,b) ((b) ? x |= flag : x &= ~flag)
#define UnsetFlag(x,flag) (x &= ~flag)=

void TiWebShell::setTiWindow(TiWindow *tiWindow)
{
	this->tiWindow = tiWindow;
	reloadTiWindow();
}

void TiWebShell::reloadTiWindow()
{
	if (tiWindow->getURL().length() > 0)
		loadURL(tiWindow->getURL().c_str());

	SetWindowText(hWnd, UTF8ToWide(tiWindow->getTitle()).c_str());

	long windowStyle = GetWindowLong(hWnd, GWL_STYLE);
	
	SetFlag(windowStyle, WS_MINIMIZEBOX, tiWindow->isMinimizable());
	SetFlag(windowStyle, WS_MAXIMIZEBOX, tiWindow->isMaximizable());

	SetFlag(windowStyle, WS_OVERLAPPEDWINDOW, tiWindow->isUsingChrome() && tiWindow->isResizable());
	SetFlag(windowStyle, WS_CAPTION, tiWindow->isUsingChrome());
	
	SetWindowLong(hWnd, GWL_STYLE, windowStyle);

	UINT flags = SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED;

	sizeTo(tiWindow->getX(), tiWindow->getY(), tiWindow->getWidth(), tiWindow->getHeight(), flags);
	SetLayeredWindowAttributes(hWnd, 0, (BYTE)floor(tiWindow->getTransparency()*255), LWA_ALPHA);
}


void TiWebShell::open() {
	static bool windowClassInitialized = false;
	if (!windowClassInitialized) {
		TiWebShell::initWindowClass();
		windowClassInitialized = true;
	}

	if (hWnd == NULL)
		createWindow();

	ShowWindow(hWnd, SW_SHOW);
	ShowWindow(host->window_handle(), SW_SHOW);
}

void TiWebShell::loadURL(const char* url) {
	if (currentURL != url) {
		currentURL = url;

		WebRequest *request = WebRequest::Create(GURL(url));
		WebFrame *frame = host->webview()->GetMainFrame();
		frame->LoadRequest(request);

		host->webview()->SetFocusedFrame(frame);

		SetFocus(host->window_handle());
		ShowWindow(host->window_handle(), SW_SHOW);
	}
}


void TiWebShell::sizeTo(int x, int y, int width, int height, UINT flags) {
	RECT rc, rw;
	GetClientRect(hWnd, &rc);
	GetWindowRect(hWnd, &rw);

	int client_width = rc.right - rc.left;
	int window_width = rw.right - rw.left;
	window_width = (window_width - client_width) + width;

	int client_height = rc.bottom - rc.top;
	int window_height = rw.bottom - rw.top;
	window_height = (window_height - client_height) + height;

	SetWindowPos(hWnd, NULL, x, y, window_width, window_height, flags);
}

void TiWebShell::resizeHost() {
	RECT rc;
	GetClientRect(hWnd, &rc);

	MoveWindow(host->window_handle(), 0, 0, rc.right, rc.bottom, TRUE);
}

void TiWebShell::include(std::string& relativePath)
{
	std::string absolutePath;

	if (relativePath.find_first_of("://") != std::string::npos) {
		absolutePath = WideToUTF8(TiURL::getPathForURL(GURL(relativePath)));
	}
	else {
		absolutePath = WideToUTF8(tiAppConfig->getResourcePath());
		absolutePath += "\\";
		absolutePath += relativePath;
	}
	std::ifstream in(absolutePath.c_str());
	std::string s, line;
	while(getline(in, line)) {
		s += line + "\n";
	}

	WebView *webview = getHost()->webview();
	webview->GetMainFrame()->ExecuteJavaScript(s, absolutePath);
}

void TiWebShell::showWindow(int nCmdShow) {
	ShowWindow(hWnd, nCmdShow);
}

std::string TiWebShell::getTitle() {
	wchar_t buffer[2049];
	GetWindowText(this->hWnd, buffer, 2048);

	std::string result;
	result.assign(WideToUTF8(buffer));

	return result;
}

void TiWebShell::setTitle(std::string title) {
	SetWindowText(this->hWnd, UTF8ToWide(title).c_str());
}

void TiWebShell::close() {
	CloseWindow(hWnd);
}
