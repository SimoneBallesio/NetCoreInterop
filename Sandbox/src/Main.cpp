#include <Core/Definitions.hpp>
#include <Core/HostedAssembly.hpp>

#include <NetCore/NetCoreController.hpp>

#include <NetCore/Api/ExampleApi.hpp>

typedef void (INTEROP_DELEGATE_CALLTYPE *PrintObjPropertiesFn)(void*);
typedef void (INTEROP_DELEGATE_CALLTYPE *PassObjectToHostFn)();
typedef void (INTEROP_DELEGATE_CALLTYPE *DelegateRoundaboutFn)();
typedef void (INTEROP_DELEGATE_CALLTYPE* ReadObjectFromSharedMemoryFn)(u32);
typedef void (INTEROP_DELEGATE_CALLTYPE* WriteObjectToSharedMemoryFn)(u32);

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
	if (success) success = controller.LoadAssemblyFunction("PassObjectToHost", "Interop.Core.Examples.EntryPoint", &interopCore);
	if (success) success = controller.LoadAssemblyFunction("DelegateRoundabout", "Interop.Core.Examples.EntryPoint", &interopCore);
	if (success) success = controller.LoadAssemblyFunction("ReadObjectFromSharedMemory", "Interop.Core.Examples.EntryPoint", &interopCore);
	if (success) success = controller.LoadAssemblyFunction("WriteObjectToSharedMemory", "Interop.Core.Examples.EntryPoint", &interopCore);
	if (success) success = controller.CloseContext();

	if (!success) return 1;

	auto PrintObjProperties = interopCore.GetFunction<PrintObjPropertiesFn>("PrintObjProperties");
	auto PassObjectToHost = interopCore.GetFunction<PassObjectToHostFn>("PassObjectToHost");
	auto DelegateRoundabout = interopCore.GetFunction<DelegateRoundaboutFn>("DelegateRoundabout");
	auto ReadObjectFromSharedMemory = interopCore.GetFunction<ReadObjectFromSharedMemoryFn>("ReadObjectFromSharedMemory");
	auto WriteObjectToSharedMemory = interopCore.GetFunction<WriteObjectToSharedMemoryFn>("WriteObjectToSharedMemory");

	Interop::NetCore::Api::CustomObject exampleObj = {};
	PrintObjProperties((void*)&exampleObj);
	PassObjectToHost();
	DelegateRoundabout();

	controller.Write(exampleObj, 1);
	ReadObjectFromSharedMemory(1);

	WriteObjectToSharedMemory(0);
	controller.Read(exampleObj, 0);
	PrintObjProperties((void*)&exampleObj);

	return 0;
}