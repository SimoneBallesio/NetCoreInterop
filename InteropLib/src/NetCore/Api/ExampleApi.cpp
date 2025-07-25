#include "Core/Definitions.hpp"

#include "NetCore/Api/ExampleApi.hpp"

#include <cstdio>

namespace Interop::NetCore::Api
{

	void PrintHostedObjProperties(void* obj)
	{
		CustomObject* customObj = static_cast<CustomObject*>(obj);
		printf("[C++] PrintHostedObjProperties: TextProperty=\"%s\"; DoubleProperty=%f\n", customObj->TextProperty, customObj->DoubleProperty);
	}

	void ProcessCustomObject(void* obj, ParseCustomObjectFn callback)
	{
		callback(obj);
	}

}