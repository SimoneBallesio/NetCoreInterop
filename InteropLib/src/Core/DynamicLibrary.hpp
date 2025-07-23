#pragma once

#include <unordered_map>

namespace Interop
{

	class DynamicLibrary final
	{
	public:
		DynamicLibrary(const char* name, const char* path = "./") : Name(name), Path(path) {}
		DynamicLibrary(DynamicLibrary&) = delete;

		~DynamicLibrary() = default;

		const char* Name;
		const char* Path;

		void* Binaries = nullptr;
		std::unordered_map<const char*, void*> Functions = {};

		DynamicLibrary& operator=(DynamicLibrary&) = delete;

	private:
		DynamicLibrary() = default;
	};

}