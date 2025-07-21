#include "Core/Definitions.hpp"

#ifdef INTEROP_PLATFORM_WIN32

#include "Core/DynamicLibrary.hpp"

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
			printf("DynamicLibrary object not initialized. Aborting load of library \"%s\"", name);
			return false;
		}

		if (library->Binaries != nullptr) [[unlikely]]
		{
			printf("Dynamic library \"%s\" already loaded in memory.", name);
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
			printf("Unable to locate library \"%s\"", name);
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
			printf("Unable to unload library function, Dynamic Library object not initialized.");
			return false;
		}

		if (library->Binaries == nullptr) [[unlikely]]
		{
			printf("Unable to unload library \"%s\"", library->Name);
			return false;
		}

		if (library->Functions.find(name) != library->Functions.end())
		{
			printf("The function pointer for function \"%s\" has already been loaded.", name);
			return true;
		}

		FARPROC fnPtr = GetProcAddress(static_cast<HMODULE>(library->Binaries), name);

		if (fnPtr == nullptr)
		{
			printf("Unable to load function \"%s\" from library \"%s\"", name, library->Name);
			return false;
		}

		library->Functions[name] = static_cast<void*>(fnPtr);

		return true;
	}

	b8 UnloadLibrary(DynamicLibrary* library)
	{
		if (library == nullptr) [[unlikely]]
		{
			printf("Unable to unload library, DynamicLibrary object not initialized.");
			return false;
		}

		if (library->Binaries == nullptr) [[unlikely]]
		{
			printf("Unable to unload library \"%s\"", library->Name);
			return false;
		}

		BOOL result = FreeLibrary(static_cast<HMODULE>(library->Binaries));

		if (!result)
		{
			printf("Unloading of library \"%s\" failed.", library->Name);
			return false;
		}

		memset(library, 0, sizeof(DynamicLibrary));

		return true;
	}

}

#undef DYNAMIC_LIBRARY_PREFIX
#undef DYNAMIC_LIBRARY_EXTENSION

#endif