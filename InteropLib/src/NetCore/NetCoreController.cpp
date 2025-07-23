#include "Core/Definitions.hpp"
#include "Core/DynamicLibrary.hpp"
#include "Core/HostedAssembly.hpp"

#include "NetCore/NetCoreController.hpp"
#include "NetCore/NetCoreVersion.hpp"

#include "Platform/Platform.hpp"

#include <hostfxr.h>
#include <coreclr_delegates.h>

#include <vector>
#include <filesystem>

#ifdef INTEROP_PLATFORM_UNIX
#define INTEROP_PATH_DELIMITER ":"

#else
#define INTEROP_PATH_DELIMITER ";"
#endif

#define INTEROP_HOSTFXR_INIT_FN_NAME "hostfxr_initialize_for_runtime_config"
#define INTEROP_HOSTFXR_GET_DELEGATE_FN_NAME "hostfxr_get_runtime_delegate"
#define INTEROP_HOSTFXR_CLOSE_FN_NAME "hostfxr_close"

namespace Interop::NetCore
{

	b8 LoadHostfxr(DynamicLibrary* hostfxr, const char* version);
	b8 ValidateHostfxrPath(const char* path, NetCoreVersion& version);

	Controller::Controller(const char* version) : m_TargetVersion(version)
	{
		m_Hostfxr = new DynamicLibrary("hostfxr");
	}

	Controller::~Controller()
	{
		if (m_Hostfxr != nullptr)
		{
			delete m_Hostfxr;
			m_Hostfxr = nullptr;
		}
	}

	b8 Controller::Init()
	{
		if (!LoadHostfxr(m_Hostfxr, m_TargetVersion))
		{
			printf("%s\n", "Unable to load hostfxr. Please check or repair your .NET installation.");
			return false;
		}

		b8 success = Platform::LoadLibraryFunction(INTEROP_HOSTFXR_INIT_FN_NAME, m_Hostfxr);
		if (success) success = Platform::LoadLibraryFunction(INTEROP_HOSTFXR_GET_DELEGATE_FN_NAME, m_Hostfxr);
		if (success) success = Platform::LoadLibraryFunction(INTEROP_HOSTFXR_CLOSE_FN_NAME, m_Hostfxr);

		if (!success)
		{
			printf("%s\n", "Unable to load the necessary function pointers from hostfxr");
			return false;
		}

		return true;
	}

	b8 LoadHostfxr(DynamicLibrary* hostfxr, const char* version)
	{
		const char* dotnetRoot = getenv("DOTNET_ROOT");
		const char* path = getenv("PATH");

		char pathStr[strlen(path) + 1];
		strcpy(pathStr, path);

		const char* delimiter = INTEROP_PATH_DELIMITER;
		char* t = strtok(pathStr, delimiter);

		std::vector<const char*> candidatePaths;
		u16 maxPathLength = 0;

		if (dotnetRoot != nullptr)
			candidatePaths.push_back(dotnetRoot);

		while (t != nullptr)
		{
			if (strlen(t) > 0)
				candidatePaths.push_back(t);

			maxPathLength = std::max(maxPathLength, static_cast<u16>(strlen(t)));
			t = strtok(nullptr, delimiter);
		}

		const char* internalPath = "/host/fxr/";
		maxPathLength += static_cast<u16>(strlen(internalPath));
		std::vector<char> buffer(maxPathLength + 1);

		NetCoreVersion requestedVersion(version);
		bool found = false;

		for (const auto& p : candidatePaths)
		{
			snprintf(buffer.data(), maxPathLength + 1, "%s%s", p, internalPath);

			if (!ValidateHostfxrPath(buffer.data(), requestedVersion))
				continue;

			u16 basePathSize = static_cast<u16>(strlen(buffer.data()));
			u16 versionStrSize;

			requestedVersion.ToString(nullptr, &versionStrSize);

			if (versionStrSize == 0) [[unlikely]]
			{
				continue;
			}

			buffer.resize(strlen(buffer.data()) + versionStrSize + 1);
			requestedVersion.ToString(buffer.data() + basePathSize, &versionStrSize);
			found = true;

			break;
		}

		if (!found) return false;

		return Platform::LoadLibrary("hostfxr", buffer.data(), hostfxr);
	}

	b8 ValidateHostfxrPath(const char* path, NetCoreVersion& version)
	{
		if (path == nullptr) [[unlikely]]
		{
			printf("%s\n", "Empty path passed to hostfxr resolver - check your PATH environment variable.");
			return false;
		}

		if (!std::filesystem::exists(path))
			return false;

		auto children = std::filesystem::directory_iterator(path);
		NetCoreVersion bestCandidate = {};

		for (const auto& entry : children)
		{
			if (!entry.is_directory()) continue;
			auto versionStr = entry.path().filename();

			NetCoreVersion v(versionStr.c_str());

			if (v < version) continue;

			else if (v == version)
				return true;

			if (v > bestCandidate)
			{
				bestCandidate.Version.Version = v.Version.Version;
				bestCandidate.Type = v.Type;
				bestCandidate.Svn.Version = v.Svn.Version;
			}
		}

		if (version.IsEmpty())
		{
			version.Version.Version = bestCandidate.Version.Version;
			version.Type = bestCandidate.Type;
			version.Svn.Version = bestCandidate.Svn.Version;

			return true;
		}

		return false;
	}

}

#undef INTEROP_HOSTFXR_CLOSE_FN_NAME
#undef INTEROP_HOSTFXR_GET_DELEGATE_FN_NAME
#undef INTEROP_HOSTFXR_INIT_FN_NAME

#undef INTEROP_PATH_DELIMITER