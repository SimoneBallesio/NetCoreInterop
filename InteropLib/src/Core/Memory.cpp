#include "Core/Definitions.hpp"
#include "Core/Memory.hpp"

#include "Platform/Platform.hpp"

namespace Interop::Memory
{

	State* State::s_Instance = nullptr;

	b8 Init(State** state)
	{
		if (State::s_Instance != nullptr) [[unlikely]]
		{
			printf("%s\n", "Trying to re-initialize existing Shared Memory System Aborting.");
			return false;
		}

		*state = new State();
		State::s_Instance = *state;

		State::s_Instance->Allocation = new SharedBuffer();
		State::s_Instance->Allocation->Name = "Controller";
		State::s_Instance->Allocation->Size = 8192;

		b8 success = Platform::OpenOrCreateMemoryMap(State::s_Instance->Allocation);

		if (!success)
		{
			printf("%s\n", "Unable to create shared memory area for inter-process operability");

			delete State::s_Instance->Allocation;
			delete State::s_Instance;

			State::s_Instance = nullptr;
			*state = nullptr;

			return false;
		}

		return true;
	}

	void Destroy(State** state)
	{
		if (State::s_Instance != *state) [[unlikely]]
		{
			printf("%s\n", "Trying to destroy a different Shared Memory System than the one initialized");
			delete *state;
		}

		if (State::s_Instance->Allocation != nullptr)
		{
			if (State::s_Instance->Allocation->State == INTEROP_MEMORY_MAP_STATE_OPEN)
			{
				b8 success = Platform::CloseMemoryMap(State::s_Instance->Allocation);

				if (!success)
				{
					printf("%s\n", "Unable to close properly shared memory area between controller and hosted libraries");
				}
			}

			delete State::s_Instance->Allocation;
		}

		if (!State::s_Instance->TypePools.empty())
		{
			for (auto it = State::s_Instance->TypePools.begin(); it != State::s_Instance->TypePools.end(); it++)
			{
				delete it->second;
			}

			State::s_Instance->TypePools.clear();
		}

		delete State::s_Instance;

		State::s_Instance = nullptr;
		*state = nullptr;
	}

}