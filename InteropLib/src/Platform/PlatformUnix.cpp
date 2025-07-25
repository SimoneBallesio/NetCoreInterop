#include "Core/Definitions.hpp"

#ifdef INTEROP_PLATFORM_UNIX

#include "Core/DynamicLibrary.hpp"
#include "Core/Memory.hpp"

#include "Platform/Platform.hpp"

#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>

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

		i32 fd = open(memory->Name, O_CREAT | O_RDWR, (mode_t)00700);

		if (fd == -1)
		{
			printf("Unable to get native handle for shared memory map \"%s\" (size: %d)\n", memory->Name, memory->Size);
			return false;
		}

		struct stat mapstat;
		if (-1 != fstat(fd, &mapstat) && mapstat.st_size == 0)
		{
			ftruncate(fd, memory->Size);
		}

		memory->BaseAddress = mmap(0, memory->Size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);

		if (memory->BaseAddress == MAP_FAILED)
		{
			printf("Unable to open or create shared memory map \"%s\" (size: %d)\n", memory->Name, memory->Size);
			memory->BaseAddress = nullptr;
			return false;
		}

		if (close(fd) != -1)
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

		if (munmap(memory->BaseAddress, memory->Size) == -1)
		{
			printf("Unable to close shared memory map \"%s\" (size: %d)\n", memory->Name, memory->Size);
			return false;
		}

		if (unlink(memory->Name) == -1)
		{
			printf("Unable to close shared memory map \"%s\" (size: %d)\n", memory->Name, memory->Size);
			return false;
		}

		memory->State = INTEROP_MEMORY_MAP_STATE_CLOSED;
		memory->BaseAddress = nullptr;
		memory->NativeHandle = nullptr;

		return true;
	}

}

#undef DYNAMIC_LIBRARY_EXTENSION
#undef DYNAMIC_LIBRARY_PREFIX

#endif