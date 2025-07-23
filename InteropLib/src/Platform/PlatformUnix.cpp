#include "Core/Definitions.hpp"

#ifdef INTEROP_PLATFORM_UNIX

#include "Core/DynamicLibrary.hpp"

#include "Platform/Platform.hpp"

#include <dlfcn.h>

#include <vector>

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
			printf("Dynamic Library object not initialized. Aborting load of library \"%s\"\n", name);
			return false;
		}

		if (library->Binaries != nullptr) [[unlikely]]
		{
			printf("Dynamic library \"%s\" already loaded in memory.\n", name);
			return true;
		}

		std::vector<char> assemblyPath;
		size_t assemblyPathLength = strlen(path) + strlen(DYNAMIC_LIBRARY_PREFIX) + strlen(name) + strlen(DYNAMIC_LIBRARY_EXTENSION) + 2;

		assemblyPath.resize(assemblyPathLength);
		snprintf(assemblyPath.data(), assemblyPathLength, "%s/%s%s%s", path, DYNAMIC_LIBRARY_PREFIX, name, DYNAMIC_LIBRARY_EXTENSION);

		void* binaries = dlopen(assemblyPath.data(), RTLD_NOW);

		if (library == nullptr)
		{
			printf("Unable to locate library \"%s\" in the specified location (%s), falling back to /usr/local/lib\n", name, assemblyPath.data());

			assemblyPathLength = strlen("/usr/local/lib/") + strlen(DYNAMIC_LIBRARY_PREFIX) + strlen(name) + strlen(DYNAMIC_LIBRARY_EXTENSION) + 1;

			assemblyPath.resize(assemblyPathLength);
			assemblyPath.clear();
			snprintf(assemblyPath.data(), assemblyPathLength, "/usr/local/lib/%s%s%s", DYNAMIC_LIBRARY_PREFIX, name, DYNAMIC_LIBRARY_EXTENSION);

			binaries = dlopen(assemblyPath.data(), RTLD_NOW);
		}

		if (library == nullptr)
		{
			printf("Unable to locate library \"%s\"", name);
			return false;
		}

		library->Name = name;
		library->Binaries = binaries;

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

		void* fnPtr = dlsym(library->Binaries, name);

		if (fnPtr == nullptr)
		{
			printf("Unable to load function \"%s\" from library \"%s\"\n", name, library->Name);
			return false;
		}

		library->Functions[name] = fnPtr;

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

		i32 result = dlclose(library->Binaries);

		if (result != 0)
		{
			printf("Unloading of library \"%s\" failed.\n", library->Name);
			return false;
		}

		memset(library, 0, sizeof(DynamicLibrary));

		return true;
	}

}

#undef DYNAMIC_LIBRARY_EXTENSION
#undef DYNAMIC_LIBRARY_PREFIX

#endif