#pragma once

#include "Core/Definitions.hpp"

namespace Interop
{

	class DynamicLibrary;
	class HostedAssembly;

}

namespace Interop::NetCore
{

	class Controller final
	{
	public:
		INTEROP_API Controller(const char* version = nullptr);
		Controller(Controller&) = delete;

		INTEROP_API ~Controller();

		INTEROP_API b8 Init();

		Controller& operator=(Controller&) = delete;

	private:
		DynamicLibrary* m_Hostfxr = nullptr;
		const char* m_TargetVersion;

		Controller() = default;
	};

}