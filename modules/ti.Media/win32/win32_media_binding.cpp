/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "win32_media.h"

namespace ti
{
	Win32MediaBinding::Win32MediaBinding(SharedBoundObject global) : MediaBinding(global)
	{
	}
	Win32MediaBinding::~Win32MediaBinding()
	{
	}
	void Win32MediaBinding::Beep()
	{
		MessageBeep(MB_OK);
	}
	SharedBoundObject Win32MediaBinding::CreateSound(std::string& url)
	{
		return new Win32Sound(url);
	}
}