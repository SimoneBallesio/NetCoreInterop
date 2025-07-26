#pragma once

#include "Core/Definitions.hpp"

#include <typeindex>
#include <unordered_map>

namespace Interop::Memory
{

	enum MemoryMapState
	{
		INTEROP_MEMORY_MAP_STATE_CLOSED = 0,
		INTEROP_MEMORY_MAP_STATE_OPEN = 1,
	};

	struct SharedBuffer
	{
		MemoryMapState State = INTEROP_MEMORY_MAP_STATE_CLOSED;
		const char* Name;

		u32 Size = 0;
		u32 ReservedSize = 0;

		void* NativeHandle = nullptr;
		void* BaseAddress = nullptr;
	};

	struct SharedBlock
	{
		void* BaseAddress = nullptr;
		u32 Offset = 0;
		u32 Capacity = 0;
		u32 Size = 0;
	};

	struct State
	{
		SharedBuffer* Allocation = nullptr;
		std::unordered_map<std::type_index, SharedBlock*> TypePools = {};

		INTEROP_API static State* s_Instance;
	};

	INTEROP_API b8 Init(State** state);
	INTEROP_API void Destroy(State** state);

	template <typename T>
	INTEROP_API SharedBlock* GetOrCreateBlock()
	{
		auto& alloc = State::s_Instance->Allocation;

		if (alloc->State == INTEROP_MEMORY_MAP_STATE_CLOSED) [[unlikely]]
		{
			printf("%s\n", "Unable to use shared memory as it has not been opened");
			return nullptr;
		}

		auto& pools = State::s_Instance->TypePools;

		if (pools.find(typeid(T)) != pools.end())
			return pools[typeid(T)];

		u32 blockSize = INTEROP_ALIGNED_SIZE(1000 * sizeof(T));

		if (alloc->ReservedSize + blockSize > alloc->Size)
		{
			blockSize = alloc->Size - alloc->ReservedSize;
			if (blockSize == 0) return nullptr;
		}

		void* baseAddress = (void*)((char*)alloc->BaseAddress + alloc->ReservedSize);
		u32 capacity = static_cast<u32>(floor(blockSize / sizeof(T)));

		SharedBlock* block = new SharedBlock();

		block->BaseAddress = baseAddress;
		block->Offset = alloc->ReservedSize;
		block->Capacity = capacity;
		block->Size = blockSize;

		pools[typeid(T)] = block;
		alloc->ReservedSize += blockSize;

		return block;
	}

	template <typename T>
	INTEROP_API T* Get(u32 index)
	{
		SharedBlock* block = GetOrCreateBlock<T>();

		if (block == nullptr)
		{
			printf("%s\n", "Unable to retrieve a shared memory block related to the specified type");
			return nullptr;
		}

		if (index > block->Capacity)
		{
			printf("%s\n", "The index provided is outside the range of the related shared memory block");
			return nullptr;
		}

		T* buffer = static_cast<T*>(block->BaseAddress);
		return &buffer[index];
	}

	template <typename T>
	INTEROP_API void Set(u32 index, const T& value)
	{
		SharedBlock* block = GetOrCreateBlock<T>();

		if (block == nullptr)
		{
			printf("%s\n", "Unable to retrieve a shared memory block related to the specified type");
			return;
		}

		if (index > block->Capacity)
		{
			printf("%s\n", "The index provided is outside the range of the related shared memory block");
			return;
		}

		T* buffer = reinterpret_cast<T*>(block->BaseAddress);
		memcpy(&buffer[index], &value, sizeof(T));
	}

}