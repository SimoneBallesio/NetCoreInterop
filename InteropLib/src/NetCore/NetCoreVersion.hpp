#pragma once

#include "Core/Definitions.hpp"

#include <string>

namespace Interop::NetCore
{

	enum NetCoreReleaseType : u16
	{
		INTEROP_NETCORE_RELEASE_TYPE_RELEASE = 0,
		INTEROP_NETCORE_RELEASE_TYPE_PREVIEW = 1,
		INTEROP_NETCORE_RELEASE_TYPE_RC = 2,
	};

	#pragma pack(push, 1)
	union AssemblyVersion
	{
		u64 Version = 0;

		struct
		{
			unsigned int Patch : 16;
			unsigned int Minor : 16;
			unsigned int Major : 16;
		};
	};
	#pragma pack(pop)

	struct NetCoreVersion
	{
		NetCoreVersion() = default;
		NetCoreVersion(const char* version);
		NetCoreVersion(u16 major, u16 minor, u16 patch);
		
		~NetCoreVersion() = default;

		NetCoreReleaseType Type = INTEROP_NETCORE_RELEASE_TYPE_RELEASE;
		AssemblyVersion Version = {};
		AssemblyVersion Svn = {};

		INTEROP_INLINE bool IsEmpty() const;

		void ToString(char* buffer, u16* size) const;

		INTEROP_INLINE bool operator==(const NetCoreVersion& other) const;
		INTEROP_INLINE bool operator!=(const NetCoreVersion& other) const;

		INTEROP_INLINE bool operator>(const NetCoreVersion& other) const;
		INTEROP_INLINE bool operator<(const NetCoreVersion& other) const;

		INTEROP_INLINE bool operator>=(const NetCoreVersion& other) const;
		INTEROP_INLINE bool operator<=(const NetCoreVersion& other) const;
	};

}

namespace Interop::NetCore
{

	INTEROP_INLINE bool NetCoreVersion::IsEmpty() const
	{
		return Version.Version == 0 && Type == 0 && Svn.Version == 0;
	}

	INTEROP_INLINE bool NetCoreVersion::operator==(const NetCoreVersion& other) const
	{
		return Version.Version == other.Version.Version &&
			Type == other.Type &&
			Svn.Version == other.Svn.Version;
	}

	INTEROP_INLINE bool NetCoreVersion::operator!=(const NetCoreVersion& other) const
	{
		return Version.Version != other.Version.Version ||
			Type != other.Type ||
			Svn.Version != other.Svn.Version;
	}

	INTEROP_INLINE bool NetCoreVersion::operator>(const NetCoreVersion& other) const
	{
		if (Version.Version > other.Version.Version)
			return true;

		if (Version.Version == other.Version.Version)
		{
			if (Type < other.Type)
				return true;

			if (Type == other.Type)
				return Svn.Version > other.Svn.Version;
		}

		return false;
	}

	INTEROP_INLINE bool NetCoreVersion::operator<(const NetCoreVersion& other) const
	{
		if (Version.Version < other.Version.Version)
			return true;

		if (Version.Version == other.Version.Version)
		{
			if (Type > other.Type)
				return true;

			if (Type == other.Type)
				return Svn.Version < other.Svn.Version;
		}

		return false;
	}

	INTEROP_INLINE bool NetCoreVersion::operator>=(const NetCoreVersion& other) const
	{
		if (Version.Version > other.Version.Version)
			return true;

		if (Version.Version == other.Version.Version)
		{
			if (Type < other.Type)
				return true;

			if (Type == other.Type)
				return Svn.Version >= other.Svn.Version;
		}

		return false;
	}

	INTEROP_INLINE bool NetCoreVersion::operator<=(const NetCoreVersion& other) const
	{
		if (Version.Version < other.Version.Version)
			return true;

		if (Version.Version == other.Version.Version)
		{
			if (Type > other.Type)
				return true;

			if (Type == other.Type)
				return Svn.Version <= other.Svn.Version;
		}

		return false;
	}
	
}