#include <Core/HostedAssembly.hpp>

#include <NetCore/NetCoreController.hpp>

int main(int argc, char* argv[])
{
	Interop::NetCore::Controller controller = Interop::NetCore::Controller("9.0.0");

	if (!controller.Init())
	{
		return 1;
	}

	return 0;
}