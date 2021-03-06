/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../ui_module.h"
#include "win32_menu_item_impl.h"

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <string>
#include "win32_tray_item.h"

namespace ti
{
	HMENU Win32UIBinding::contextMenuInUseHandle = NULL;


	Win32UIBinding::Win32UIBinding(Host *host) : UIBinding(host)
	{

		// We're not using registration-free COM because it requires that
		// the DLL be in a subdirectory of the exe. Use Activation Context
		// API instead. We just need to point it to the .manifest file as below.
		ACTCTX actctx; 
		ZeroMemory(&actctx, sizeof(actctx)); 
		actctx.cbSize = sizeof(actctx); 

		std::string source = host->GetRuntimePath();
		source = FileUtils::Join(source.c_str(), "WebKit.manifest", NULL);
		actctx.lpSource = source.c_str(); // Path to the Side-by-Side Manifest File 
		this->pActCtx = CreateActCtx(&actctx); 
		ActivateActCtx(pActCtx, &this->lpCookie);
	}

	Win32UIBinding::~Win32UIBinding()
	{
   		DeactivateActCtx(0, this->lpCookie); 
		ReleaseActCtx(this->pActCtx);
	}

	SharedUserWindow Win32UIBinding::CreateWindow(
		WindowConfig* config,
		SharedUserWindow& parent)
	{
		UserWindow* w = new Win32UserWindow(this, config, parent);
		return w->GetSharedPtr();
	}

	void Win32UIBinding::ErrorDialog(std::string msg)
	{
		MessageBox(NULL,msg.c_str(),"Application Error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
		UIBinding::ErrorDialog(msg);
	}

	SharedPtr<MenuItem> Win32UIBinding::CreateMenu(bool trayMenu)
	{
		SharedPtr<MenuItem> menu = new Win32MenuItemImpl(NULL);
		return menu;
	}

	void Win32UIBinding::SetMenu(SharedPtr<MenuItem>)
	{
		// Notify all windows that the app menu has changed.
		std::vector<SharedUserWindow>& windows = this->GetOpenWindows();
		std::vector<SharedUserWindow>::iterator i = windows.begin();
		while (i != windows.end())
		{
			SharedPtr<Win32UserWindow> wuw = (*i).cast<Win32UserWindow>();
			if (!wuw.isNull())
				wuw->AppMenuChanged();

			i++;
		}
	}

	void Win32UIBinding::SetContextMenu(SharedPtr<MenuItem> menu)
	{
		SharedPtr<Win32MenuItemImpl> menu_new = menu.cast<Win32MenuItemImpl>();

		// if same menu, just return
		if ((menu.isNull() && contextMenuInUse.isNull()) || (menu_new == contextMenuInUse))
		{
			return;
		}

		// delete old menu if available
		if(! contextMenuInUse.isNull())
		{
			contextMenuInUse->ClearRealization(contextMenuInUseHandle);
			contextMenuInUseHandle = NULL;
		}

		contextMenuInUse = menu_new;

		// create new menu if needed
		if(! contextMenuInUse.isNull())
		{
			contextMenuInUseHandle = contextMenuInUse->GetMenu();
		}
	}

	void Win32UIBinding::SetIcon(SharedString icon_path)
	{
		// Notify all windows that the app icon has changed
		// TODO this kind of notification should really be placed in UIBinding..
		std::vector<SharedUserWindow>& windows = this->GetOpenWindows();
		std::vector<SharedUserWindow>::iterator i = windows.begin();
		while (i != windows.end())
		{
			SharedPtr<Win32UserWindow> wuw = (*i).cast<Win32UserWindow>();
			if (!wuw.isNull())
				wuw->AppIconChanged();

			i++;
		}
	}

	SharedPtr<TrayItem> Win32UIBinding::AddTray(
		SharedString icon_path,
		SharedKMethod cb)
	{
		SharedPtr<TrayItem> trayItem = new Win32TrayItem(icon_path, cb);
		return trayItem;
	}

	long Win32UIBinding::GetIdleTime()
	{
		LASTINPUTINFO lii;
		memset(&lii, 0, sizeof(lii));

		lii.cbSize = sizeof(lii);
		::GetLastInputInfo(&lii);

		DWORD currentTickCount = GetTickCount();
		long idleTicks = currentTickCount - lii.dwTime;

		return (int)idleTicks;
	}
}
