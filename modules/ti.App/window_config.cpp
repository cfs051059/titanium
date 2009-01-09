/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "app_config.h"
#include "window_config.h"

using namespace ti;

int WindowConfig::DEFAULT_POSITION = -1;

void WindowConfig::SetDefaults ()
{
	maximizable = minimizable = closeable = resizable = true;
	usingChrome = usingScrollbars = fullscreen = false;
	transparency = 1.0;
	width = 800;
	height = 600;
	x = y = WindowConfig::DEFAULT_POSITION;
	minWidth = minHeight = 0;
	maxWidth = maxHeight = 9000;
	url = "app://" + AppConfig::Instance()->GetAppID() + "/index.html";
	title = "Titanium Application";
	visible = true;
}

WindowConfig::WindowConfig(void* data)
{
	xmlElementPtr element = (xmlElementPtr) data;
	SetDefaults();

	xmlNodePtr child = element->children;
	while (child != NULL) {
		if (nodeNameEquals(child, "id")) {
			winid = nodeValue(child);
		}
		else if (nodeNameEquals(child, "title")) {
			title = nodeValue(child);
		}
		else if (nodeNameEquals(child, "url")) {
			url = nodeValue(child);
		}
		else if (nodeNameEquals(child, "maximizable")) {
			maximizable = boolValue(child);
		}
		else if (nodeNameEquals(child, "minimizable")) {
			minimizable =	 boolValue(child);
		}
		else if (nodeNameEquals(child, "closeable")) {
			closeable = boolValue(child);
		}
		else if (nodeNameEquals(child, "resizable")) {
			resizable = boolValue(child);
		}
		else if (nodeNameEquals(child, "fullscreen")) {
			fullscreen = boolValue(child);
		}
		else if (nodeNameEquals(child, "chrome")) {
			usingChrome = boolValue(child);
			const char * scrollbars = (const char *)xmlGetProp(child, (const xmlChar *)"scrollbars");
			if (scrollbars != NULL) {
				usingScrollbars = AppConfig::StringToBool(scrollbars);
			}
		}
		else if (nodeNameEquals(child, "transparency")) {
			transparency = (float)atof(nodeValue(child));
		}
		else if (nodeNameEquals(child, "width")) {
			width = atoi(nodeValue(child));
		}
		else if (nodeNameEquals(child, "height")) {
			height = atoi(nodeValue(child));
		}
		else if (nodeNameEquals(child, "min-width")) {
			minWidth = atoi(nodeValue(child));
		}
		else if (nodeNameEquals(child, "max-width")) {
			maxWidth = atoi(nodeValue(child));
		}
		else if (nodeNameEquals(child, "min-height")) {
			minHeight = atoi(nodeValue(child));
		}
		else if (nodeNameEquals(child, "max-height")) {
			maxHeight = atoi(nodeValue(child));
		}
		child = child->next;
	}
}

std::string WindowConfig::ToString()
{
	std::ostringstream stream;

	stream << "[WindowConfig id=" << winid
		<< ", x=" << x << ", y=" << y
		<< ", width=" << width << ", height=" << height
		<< ", url=" << url
		<< "]";

	return stream.str();
}