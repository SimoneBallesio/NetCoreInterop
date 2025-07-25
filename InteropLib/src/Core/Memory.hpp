#pragma once

#include "Core/Definitions.hpp"

namespace Interop
{

	enum MemoryMapState
	{
		INTEROP_MEMORY_MAP_STATE_CLOSED = 0,
		INTEROP_MEMORY_MAP_STATE_OPEN = 1,
	};

	struct SharedMemoryArea
	{
		MemoryMapState State = INTEROP_MEMORY_MAP_STATE_CLOSED;
		const char* Name;
		u32 Size = 0;
		void* NativeHandle = nullptr;
		void* BaseAddress = nullptr;
	};

}