//
// Copyright 2006-2008 Appcelerator, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <atlbase.h>
#include <commdlg.h>
#include <objbase.h>
#include <shlwapi.h>
#include <wininet.h>
#include <commctrl.h>
#include "ti_utils.h"
#include "base/scoped_ptr.h"
#include "webkit/glue/webpreferences.h"
#include "webkit/glue/weburlrequest.h"
#include "webkit/glue/webframe.h"
#include "webkit/glue/webview.h"
#include "webkit/glue/webview_delegate.h"
#include "webkit/glue/webwidget_delegate.h"
#include "webkit/glue/plugins/webplugin_delegate_impl.h"
#include "webkit/glue/webkit_glue.h"
#include "base/gfx/point.h"
#include "base/file_util.h"
#include "base/basictypes.h"
#include "base/resource_util.h"
#include "base/ref_counted.h"
#include "base/path_service.h"
#include "base/at_exit.h"
#include "base/process_util.h"
#include "base/message_loop.h"
#include "base/icu_util.h"
#include "net/base/net_module.h"
#include "webview_host.h"
#include "webwidget_host.h"
#include "simple_resource_loader_bridge.h"
#include "test_shell_request_context.h"

#ifdef TIDEBUG
bool ti_debugging = true;
#else
bool ti_debugging = false;
#endif

void ti_debug_internal(char *message, char* filename, int line) {
	if (ti_debugging) {
		fprintf(stderr, "[titanium %s:%d] %s\n", filename, line, message);
	}
}

WebPreferences ti_initWebPrefs() {
	WebPreferences webPrefs;

	webPrefs.standard_font_family = L"Times";
	webPrefs.fixed_font_family = L"Courier";
	webPrefs.serif_font_family = L"Times";
	webPrefs.sans_serif_font_family = L"Helvetica";
	// These two fonts are picked from the intersection of
	// Win XP font list and Vista font list :
	//   http://www.microsoft.com/typography/fonts/winxp.htm 
	//   http://blogs.msdn.com/michkap/archive/2006/04/04/567881.aspx
	// Some of them are installed only with CJK and complex script
	// support enabled on Windows XP and are out of consideration here. 
	// (although we enabled both on our buildbots.)
	// They (especially Impact for fantasy) are not typical cursive
	// and fantasy fonts, but it should not matter for layout tests
	// as long as they're available.
	webPrefs.cursive_font_family = L"Comic Sans MS";
	webPrefs.fantasy_font_family = L"Impact";
	webPrefs.default_encoding = L"ISO-8859-1";
	webPrefs.default_font_size = 16;
	webPrefs.default_fixed_font_size = 13;
	webPrefs.minimum_font_size = 1;
	webPrefs.minimum_logical_font_size = 9;
	webPrefs.javascript_can_open_windows_automatically = true;
	webPrefs.dom_paste_enabled = true;
	webPrefs.developer_extras_enabled = false;
	webPrefs.shrinks_standalone_images_to_fit = false;
	webPrefs.uses_universal_detector = false;
	webPrefs.text_areas_are_resizable = false;
	webPrefs.java_enabled = true;
	webPrefs.allow_scripts_to_close_windows = false;

	return webPrefs;
}

void systemError(const wchar_t *message)
{
	MessageBox(NULL, message, L"Titanium Error", MB_OK | MB_ICONERROR);
}