#include "Core/Definitions.hpp"

#include "NetCore/NetCoreVersion.hpp"

#include <string>
#include <sstream>

namespace Interop::NetCore
{

	NetCoreVersion::NetCoreVersion(u16 major, u16 minor, u16 patch)
	{
		Version.Major = major;
		Version.Minor = minor;
		Version.Patch = patch;
	}

	NetCoreVersion::NetCoreVersion(const char* version)
	{
		if (version == nullptr) return;

		char versionToParse[strlen(version) + 1];
		strcpy(versionToParse, version);

		const char* delimiter = ".";
		char* t = strtok(versionToParse, delimiter);

		AssemblyVersion* currentVersion = &Version;
		u8 shift = 32;

		while (t != nullptr)
		{
			auto result = std::string(t);
			bool hasSvn = false;

			if (result.empty())
			{
				t = strtok(nullptr, delimiter);
				continue;
			}

			if (Type == INTEROP_NETCORE_RELEASE_TYPE_RELEASE)
			{
				if (result.find("-rc") != std::string::npos)
					Type = INTEROP_NETCORE_RELEASE_TYPE_RC;

				else if (result.find("-preview") > std::string::npos)
					Type = INTEROP_NETCORE_RELEASE_TYPE_PREVIEW;

				if (Type > INTEROP_NETCORE_RELEASE_TYPE_RELEASE)
				{
					result = result.substr(0, result.find('-'));
					hasSvn = true;
				}
			}

			currentVersion->Version |= (static_cast<u64>(std::stoi(result.c_str())) << shift);

			if (!hasSvn)
			{
				shift -= 16;
				t = strtok(nullptr, delimiter);
				continue;
			}

			currentVersion = &Svn;
			shift = 32;

			t = strtok(nullptr, delimiter);
		}
	}

	void NetCoreVersion::ToString(char* buffer, u16* size) const
	{
		std::stringstream ss;
		ss << Version.Major << '.' << Version.Minor << '.' << Version.Patch;

		switch (Type)
		{
			case INTEROP_NETCORE_RELEASE_TYPE_RELEASE:
				break;

			case INTEROP_NETCORE_RELEASE_TYPE_PREVIEW:
				ss << "-preview." << Svn.Major << '.' << Svn.Minor << '.' << Svn.Patch;
				break;

			case INTEROP_NETCORE_RELEASE_TYPE_RC:
				ss << "-rc." << Svn.Major << '.' << Svn.Minor << '.' << Svn.Patch;
				break;

			default:
				break;
		}

		std::string result = ss.str();
		*size = static_cast<u16>(result.length());

		if (buffer == nullptr) return;

		snprintf(buffer, *size + 1, "%s", result.c_str());
	}

}