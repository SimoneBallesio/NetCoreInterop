#pragma once

#include "Core/Definitions.hpp"

namespace Interop
{

	class DynamicLibrary;
	class HostedAssembly;

}

namespace Interop::NetCore
{

	struct NetCoreContext;

	class Controller final
	{
	public:
		INTEROP_API Controller(const char* version = nullptr);
		Controller(Controller&) = delete;

		INTEROP_API ~Controller();

		INTEROP_API b8 Init();

		INTEROP_API b8 OpenContext(HostedAssembly* assembly);
		INTEROP_API b8 CloseContext();
		INTEROP_API b8 LoadAssemblyFunction(const char* name, const char* classPath, HostedAssembly* assembly) const;

		Controller& operator=(Controller&) = delete;

	private:
		DynamicLibrary* m_Hostfxr = nullptr;
		NetCoreContext* m_CurrentContext = nullptr;

		const char* m_TargetVersion;

		Controller() = default;
	};

}