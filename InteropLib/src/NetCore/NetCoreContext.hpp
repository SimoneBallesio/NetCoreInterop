#pragma once

namespace Interop::NetCore
{

	struct NetCoreContext
	{
		void* Context = nullptr;
		void* LoadFunctionPointer = nullptr;
	};

}