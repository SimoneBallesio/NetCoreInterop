#include "Core/Definitions.hpp"

#ifdef INTEROP_PLATFORM_UNIX

#include "Core/DynamicLibrary.hpp"

#include "Platform/Platform.hpp"

#include <dlfcn.h>

#define DYNAMIC_LIBRARY_PREFIX "lib"

#ifdef INTEROP_PLATFORM_APPLE
#define DYNAMIC_LIBRARY_EXTENSION ".dylib"

#else
#define DYNAMIC_LIBRARY_EXTENSION ".so"
#endif

namespace Interop::Platform
{

	b8 LoadLibrary(const char* name, const char* path, DynamicLibrary* library)
	{
		if (library == nullptr) [[unlikely]]
		{
			printf("Dynamic Library object not initialized. Aborting load of library \"%s\"", name);
			return false;
		}

		if (library->Binaries != nullptr) [[unlikely]]
		{
			printf("Dynamic library \"%s\" already loaded in memory.", name);
			return true;
		}

		char buffer[200] = {};
		snprintf(buffer, 200, "%s%s%s%s", path, DYNAMIC_LIBRARY_PREFIX, name, DYNAMIC_LIBRARY_EXTENSION);

		void* binaries = dlopen(buffer, RTLD_NOW);

		if (library == nullptr)
		{
			printf("Unable to locate library \"%s\" in the specified location (%s), falling back to /usr/local/lib", name, buffer);

			memset(buffer, 0, 200 * sizeof(char));
			snprintf(buffer, 200, "/usr/local/lib/%s%s%s", DYNAMIC_LIBRARY_PREFIX, name, DYNAMIC_LIBRARY_EXTENSION);

			binaries = dlopen(buffer, RTLD_NOW);
		}

		if (library == nullptr)
		{
			printf("Unable to locate library \"%s\"", name);
			return false;
		}

		library->Name = name;
		library->Path = buffer;
		library->Binaries = binaries;

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

		void* fnPtr = dlsym(library->Binaries, name);

		if (fnPtr == nullptr)
		{
			printf("Unable to load function \"%s\" from library \"%s\"", name, library->Name);
			return false;
		}

		library->Functions[name] = fnPtr;

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

		i32 result = dlclose(library->Binaries);

		if (result != 0)
		{
			printf("Unloading of library \"%s\" failed.", library->Name);
			return false;
		}

		memset(library, 0, sizeof(DynamicLibrary));

		return true;
	}

}

#undef DYNAMIC_LIBRARY_EXTENSION
#undef DYNAMIC_LIBRARY_PREFIX

#endif