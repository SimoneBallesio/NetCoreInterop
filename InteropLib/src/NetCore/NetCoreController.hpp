#pragma once

#include "Core/Definitions.hpp"
#include "Core/Memory.hpp"

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

		template <typename T>
		INTEROP_API b8 Write(const T& obj, u32 index);

		template <typename T>
		INTEROP_API b8 Read(T& obj, u32 index);

	private:
		DynamicLibrary* m_Hostfxr = nullptr;
		NetCoreContext* m_CurrentContext = nullptr;
		SharedMemoryArea* m_Memory = nullptr;

		const char* m_TargetVersion;

		Controller() = default;
	};

}

namespace Interop::NetCore
{

	template <typename T>
	INTEROP_INLINE b8 Controller::Write(const T& obj, u32 index)
	{
		if (m_Memory->State == INTEROP_MEMORY_MAP_STATE_CLOSED) [[unlikely]]
		{
			printf("%s\n", "Unable to write object to shared memory as it has not been opened");
			return false;
		}

		T* baseAddr = reinterpret_cast<T*>(m_Memory->BaseAddress);

		memcpy((void*)&baseAddr[index], &obj, sizeof(T));

		return true;
	}

	template <typename T>
	INTEROP_INLINE b8 Controller::Read(T& obj, u32 index)
	{
		if (m_Memory->State == INTEROP_MEMORY_MAP_STATE_CLOSED) [[unlikely]]
		{
			printf("%s\n", "Unable to write object to shared memory as it has not been opened");
			return false;
		}

		T* baseAddr = reinterpret_cast<T*>(m_Memory->BaseAddress);

		memcpy(&obj, (void*)&baseAddr[index], sizeof(T));

		return true;
	}

}