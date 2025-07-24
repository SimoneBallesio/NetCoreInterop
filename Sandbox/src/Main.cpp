#include <Core/Definitions.hpp>
#include <Core/HostedAssembly.hpp>

#include <NetCore/NetCoreController.hpp>

#include "Examples/CustomObject.hpp"

typedef void (INTEROP_DELEGATE_CALLTYPE *PrintObjPropertiesFn)(void*);

int main(int argc, char* argv[])
{
	Interop::NetCore::Controller controller = Interop::NetCore::Controller("9.0.0");

	if (!controller.Init())
	{
		return 1;
	}

	Interop::HostedAssembly interopCore("Interop.Core");

	b8 success = controller.OpenContext(&interopCore);
	if (success) success = controller.LoadAssemblyFunction("PrintObjProperties", "Interop.Core.Examples.EntryPoint", &interopCore);
	if (success) success = controller.CloseContext();

	if (!success) return 1;

	auto PrintObjProperties = interopCore.GetFunction<PrintObjPropertiesFn>("PrintObjProperties");

	CustomObject exampleObj = {};
	PrintObjProperties((void*)&exampleObj);

	return 0;
}