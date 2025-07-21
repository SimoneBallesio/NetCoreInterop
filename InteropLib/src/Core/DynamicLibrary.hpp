#pragma once

#include <unordered_map>

namespace Interop
{

	struct DynamicLibrary
	{
		const char* Name;
		const char* Path;

		void* Binaries = nullptr;
		std::unordered_map<const char*, void*> Functions = {};
	};

}