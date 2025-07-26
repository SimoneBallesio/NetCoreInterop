#pragma once

#include "Core/Definitions.hpp"

namespace Interop
{

	class DynamicLibrary;

}

namespace Interop::Memory
{

	struct SharedBuffer;

}

namespace Interop::Platform
{

	b8 LoadLibrary(const char* name, const char* path, DynamicLibrary* library);
	b8 LoadLibraryFunction(const char* name, DynamicLibrary* library);
	b8 UnloadLibrary(DynamicLibrary* library);

	b8 OpenOrCreateMemoryMap(Memory::SharedBuffer* memory);
	b8 CloseMemoryMap(Memory::SharedBuffer* memory);

}