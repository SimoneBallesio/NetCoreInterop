#pragma once

#include "Core/Definitions.hpp"

namespace Interop::NetCore::Api
{

	struct CustomObject
	{
		char TextProperty[256] = "Lorem Ipsum";
		f64 DoubleProperty = 6.5333;
	};

	typedef void (INTEROP_DELEGATE_CALLTYPE* ParseCustomObjectFn)(void* obj);

	INTEROP_C_API void PrintHostedObjProperties(void* obj);
	INTEROP_C_API void ProcessCustomObject(void* obj, ParseCustomObjectFn callback);

}