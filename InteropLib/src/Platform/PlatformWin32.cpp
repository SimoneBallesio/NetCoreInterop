#include "Core/Definitions.hpp"

#ifdef INTEROP_PLATFORM_WIN32

#include "Core/DynamicLibrary.hpp"
#include "Core/Memory.hpp"

#include "Platform/Platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>

#define DYNAMIC_LIBRARY_PREFIX ""
#define DYNAMIC_LIBRARY_EXTENSION ".dll"

namespace Interop::Platform
{

	b8 LoadLibrary(const char* name, const char* path, DynamicLibrary* library)
	{
		if (library == nullptr) [[unlikely]]
		{
			printf("DynamicLibrary object not initialized. Aborting load of library \"%s\"\n", name);
			return false;
		}

		if (library->Binaries != nullptr) [[unlikely]]
		{
			printf("Dynamic library \"%s\" already loaded in memory.\n", name);
			return true;
		}

		char buffer[200] = {};
		snprintf(buffer, 200, "%s%s%s%s", path, DYNAMIC_LIBRARY_PREFIX, name, DYNAMIC_LIBRARY_EXTENSION);

		i32 pathLength = strlen(buffer);
		i32 wcharLength = MultiByteToWideChar(CP_UTF8, 0, buffer, pathLength, nullptr, 0);

		std::wstring bufferWstr(wcharLength, 0);
		MultiByteToWideChar(CP_UTF8, 0, buffer, pathLength, &bufferWstr[0], wcharLength);

		HMODULE library = LoadLibraryW(bufferWstr);

		if (library == nullptr)
		{
			printf("Unable to locate library \"%s\"\n", name);
			return false;
		}

		library->Name = name;
		library->Path = buffer;
		library->Binaries = static_cast<void*>(library);

		return true;
	}

	b8 LoadLibraryFunction(const char* name, DynamicLibrary* library)
	{
		if (library == nullptr) [[unlikely]]
		{
			printf("%s\n", "Unable to unload library function, Dynamic Library object not initialized.");
			return false;
		}

		if (library->Binaries == nullptr) [[unlikely]]
		{
			printf("Unable to unload library \"%s\"\n", library->Name);
			return false;
		}

		if (library->Functions.find(name) != library->Functions.end())
		{
			printf("The function pointer for function \"%s\" has already been loaded.\n", name);
			return true;
		}

		FARPROC fnPtr = GetProcAddress((HMODULE)library->Binaries, name);

		if (fnPtr == nullptr)
		{
			printf("Unable to load function \"%s\" from library \"%s\"\n", name, library->Name);
			return false;
		}

		library->Functions[name] = static_cast<void*>(fnPtr);

		return true;
	}

	b8 UnloadLibrary(DynamicLibrary* library)
	{
		if (library == nullptr) [[unlikely]]
		{
			printf("%s\n", "Unable to unload library, DynamicLibrary object not initialized.");
			return false;
		}

		if (library->Binaries == nullptr) [[unlikely]]
		{
			printf("Unable to unload library \"%s\"\n", library->Name);
			return false;
		}

		BOOL result = FreeLibrary((HMODULE)library->Binaries);

		if (!result)
		{
			printf("Unloading of library \"%s\" failed.\n", library->Name);
			return false;
		}

		memset(library, 0, sizeof(DynamicLibrary));

		return true;
	}

	b8 OpenOrCreateMemoryMap(SharedMemoryArea* memory)
	{
		if (memory->Name == nullptr) [[unlikely]]
		{
			printf("%s\n", "Unable to open or create share memory map, no name has been provided");
			return false;
		}

		if (memory->State == INTEROP_MEMORY_MAP_STATE_OPEN)
		{
			printf("Shared memory map \"%s\" is already open\n", memory->Name);
			return true;
		}

		memory->Size = pow(2, ceil(log2(memory->Size) / log2(2)));

		if (memory->Size == 0)
		{
			printf("Unable to open or create shared memory map \"%s\" with a size of 0", memory->Name);
			return false;
		}

		memory->NativeHandle = (void*)OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memory->Name);

		if (memory->NativeHandle == nullptr)
		{
			memory->NativeHandle = (void*)CreateFileMapping
			(
				INVALID_HANDLE_VALUE,
				nullptr,
				PAGE_READWRITE,
				0,
				memory->Size,
				memory->Name
			);
		}

		if (memory->NativeHandle == nullptr)
		{
			printf("Unable to get native handle for shared memory map \"%s\" (size: %d)\n", memory->Name, memory->Size);
			return false;
		}

		memory->BaseAddress = MapViewOfFile((HANDLE)memory->NativeHandle, FILE_MAP_ALL_ACCESS, 0, 0, memory->Size);

		if (memory->BaseAddress != nullptr)
		{
			memory->State = INTEROP_MEMORY_MAP_STATE_OPEN;
			return true;
		}

		printf("Unable to open or create shared memory map \"%s\" (size: %d)\n", memory->Name, memory->Size);
		return false;
	}

	b8 CloseMemoryMap(SharedMemoryArea* memory)
	{
		if (memory->State != INTEROP_MEMORY_MAP_STATE_OPEN)
		{
			printf("Shared memory map \"%s\" is already closed\n", memory->Name);
			return true;
		}

		bool success = UnmapViewOfFile(memory->BaseAddress);
		if (success) success = CloseHandle((HANDLE)memory->NativeHandle);

		if (success)
		{
			memory->State = INTEROP_MEMORY_MAP_STATE_CLOSED;
			memory->BaseAddress = nullptr;
			memory->NativeHandle = nullptr;

			return true;
		}

		printf("Unable to close shared memory map \"%s\" (size: %d)\n", memory->Name, memory->Size);
		return false;
	}

}

#undef DYNAMIC_LIBRARY_PREFIX
#undef DYNAMIC_LIBRARY_EXTENSION

#endif