#include "Core/Definitions.hpp"
#include "Core/DynamicLibrary.hpp"
#include "Core/HostedAssembly.hpp"

#include "NetCore/NetCoreContext.hpp"
#include "NetCore/NetCoreController.hpp"
#include "NetCore/NetCoreVersion.hpp"

#include "Platform/Platform.hpp"

#include <hostfxr.h>
#include <coreclr_delegates.h>

#include <vector>
#include <filesystem>
#include <string>

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

	b8 Controller::OpenContext(HostedAssembly* assembly)
	{
		if (assembly == nullptr) [[unlikely]]
		{
			printf("%s\n", "Unable to open a .NET context, no assembly specified");
			return false;
		}

		if (m_CurrentContext != nullptr)
		{
			if (m_CurrentContext->Context != nullptr)
			{
				if (m_CurrentContext->LoadFunctionPointer != nullptr)
				{
					printf("%s\n", ".NET context already initialized");
					return true;
				}

				printf("%s\n", "Invalid .NET context, please close it fully before reusing it");
				return false;
			}
		}

		m_CurrentContext = new NetCoreContext();

		if (m_Hostfxr->Binaries == nullptr || m_Hostfxr->Functions.empty()) [[unlikely]]
		{
			printf("%s\n", "Unable to open a .NET context, hostfxr has not been initialized");
			delete m_CurrentContext;

			return false;
		}

		const auto initForRuntime = (hostfxr_initialize_for_runtime_config_fn)m_Hostfxr->Functions[INTEROP_HOSTFXR_INIT_FN_NAME];
		const auto close = (hostfxr_close_fn)m_Hostfxr->Functions[INTEROP_HOSTFXR_CLOSE_FN_NAME];

		std::string runtimeCfgPath = std::string(assembly->Path);
		runtimeCfgPath += assembly->Name;
		runtimeCfgPath += ".runtimeconfig.json";

		i32 result = initForRuntime(runtimeCfgPath.c_str(), nullptr, &m_CurrentContext->Context);

		if (result != 0)
		{
			printf("Unable to load or parse .NET runtime configuration file (Path: \"%s\")\n", runtimeCfgPath.c_str());

			close(m_CurrentContext->Context);
			delete m_CurrentContext;

			return false;
		}

		const auto getRuntimeDelegate = (hostfxr_get_runtime_delegate_fn)m_Hostfxr->Functions[INTEROP_HOSTFXR_GET_DELEGATE_FN_NAME];
		result = getRuntimeDelegate(m_CurrentContext->Context, hdt_load_assembly_and_get_function_pointer, &m_CurrentContext->LoadFunctionPointer);

		if (result != 0)
		{
			printf("%s\n", "Unable to load function to fetch C# methods. Aborting.");

			close(m_CurrentContext->Context);
			delete m_CurrentContext;

			return false;
		}

		return true;
	}

	b8 Controller::CloseContext()
	{
		if (m_CurrentContext == nullptr) [[unlikely]]
		{
			printf("%s\n", "The .NET context is already closed");
			return true;
		}

		if (m_Hostfxr->Binaries == nullptr || m_Hostfxr->Functions.empty()) [[unlikely]]
		{
			printf("%s\n", "Unable to open a .NET context, hostfxr has not been initialized");
			return false;
		}

		const auto close = (hostfxr_close_fn)m_Hostfxr->Functions[INTEROP_HOSTFXR_CLOSE_FN_NAME];

		close(m_CurrentContext->Context);
		delete m_CurrentContext;

		return true;
	}

	b8 Controller::LoadAssemblyFunction(const char* name, const char* classPath, HostedAssembly* assembly) const
	{
		if (name == nullptr) [[unlikely]]
		{
			printf("%s\n", "Unable to load function pointer from .NET assembly, no function name provided.");
			return false;
		}

		if (classPath == nullptr) [[unlikely]]
		{
			printf("Unable to load function \"%s\" from .NET assembly, no class path specified\n", name);
			return false;
		}

		if (assembly == nullptr) [[unlikely]]
		{
			printf("Unable to load function \"%s\" from .NET assembly, the HostedAssembly object has not been initialized\n", name);
			return false;
		}

		if (assembly->Functions.find(name) != assembly->Functions.end())
		{
			printf("Function \"%s\" already loaded for .NET assembly \"%s\"\n", name, assembly->Name);
			return true;
		}

		if (m_CurrentContext == nullptr)
		{
			printf("%s\n", ".NET context close, please open it before trying to load functions");
			return false;
		}

		std::string assemblyPath = std::string(assembly->Path);
		assemblyPath += assembly->Name;
		assemblyPath += ".dll";

		std::string qualifiedType = std::string(classPath);
		qualifiedType += ", ";
		qualifiedType += assembly->Name;

		void* fn = nullptr;
		i32 result = ((load_assembly_and_get_function_pointer_fn)m_CurrentContext->LoadFunctionPointer)
		(
			assemblyPath.c_str(),
			qualifiedType.c_str(),
			name,
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			&fn
		);

		if (result != 0) return false;

		assembly->Functions[name] = fn;

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