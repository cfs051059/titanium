/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "filesystem_module.h"
#include "filesystem_test.h"

using namespace kroll;
using namespace ti;

namespace ti
{
	KROLL_MODULE(FilesystemModule);
	
	void FilesystemModule::Initialize()
	{
		// load our variables
		this->variables = new FilesystemBinding(host->GetGlobalObject());

		// set our ti.Filesystem
		Value *value = new Value(this->variables);
		host->GetGlobalObject()->Set("Filesystem",value);
		KR_DECREF(value);
	}

	void FilesystemModule::Destroy()
	{
		KR_DECREF(this->variables);
	}
	
	void FilesystemModule::Test()
	{
	  FilesystemUnitTestSuite test;
	  test.Run(host);
	}
}
