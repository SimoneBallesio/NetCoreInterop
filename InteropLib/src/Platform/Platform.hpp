#pragma once

#include "Core/Definitions.hpp"

namespace Interop
{

	struct DynamicLibrary;

}

namespace Interop::Platform
{

	b8 LoadLibrary(const char* name, const char* path, DynamicLibrary* library);
	b8 LoadLibraryFunction(const char* name, DynamicLibrary* library);
	b8 UnloadLibrary(DynamicLibrary* library);

}