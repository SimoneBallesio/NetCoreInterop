#pragma once

#include "Core/Definitions.hpp"

#include <unordered_map>

namespace Interop
{

	class HostedAssembly final
	{
	public:
		HostedAssembly(HostedAssembly&) = delete;
		HostedAssembly(const char* name) : Name(name) {}

		~HostedAssembly() = default;

		const char* Name;
		const char* Path;

		std::unordered_map<const char*, void*> Functions = {};

		HostedAssembly& operator=(HostedAssembly&) = delete;

		template <typename T>
		INTEROP_INLINE T GetFunction(const char* name) const;

	private:
		HostedAssembly() = default;
	};

}

namespace Interop
{

	template <typename T>
	INTEROP_INLINE T HostedAssembly::GetFunction(const char* name) const
	{
		return static_cast<T>(Functions.at(name));
	}

}