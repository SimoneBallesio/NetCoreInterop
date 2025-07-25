#pragma once

#include "Core/Definitions.hpp"

namespace Interop
{

	class DynamicLibrary;
	struct SharedMemoryArea;

}

namespace Interop::Platform
{

	b8 LoadLibrary(const char* name, const char* path, DynamicLibrary* library);
	b8 LoadLibraryFunction(const char* name, DynamicLibrary* library);
	b8 UnloadLibrary(DynamicLibrary* library);

	b8 OpenOrCreateMemoryMap(SharedMemoryArea* memory);
	b8 CloseMemoryMap(SharedMemoryArea* memory);

}