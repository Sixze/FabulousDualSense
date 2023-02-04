#pragma once

#include <DualSenseWindows.h>

#include "Logging/LogMacros.h"

FABULOUSDUALSENSE_API DECLARE_LOG_CATEGORY_EXTERN(LogFabulousDualSense, Log, All)

#define DS_GET_TYPE_STRING(Type) \
	((void) sizeof UEAsserts_Private::GetMemberNameCheckedJunk(static_cast<Type*>(nullptr)), TEXT(#Type))

namespace DsUtility
{
	FABULOUSDUALSENSE_API constexpr const TCHAR* ReturnValueToString(DS5W_ReturnValue ReturnValue);

	FABULOUSDUALSENSE_API constexpr const TCHAR* DeviceConnectionToString(DS5W::DeviceConnection DeviceConnection);
}
