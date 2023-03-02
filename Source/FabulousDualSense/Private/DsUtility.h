#pragma once

#include <DualSenseWindows.h>

#include "Logging/LogMacros.h"

FABULOUSDUALSENSE_API DECLARE_LOG_CATEGORY_EXTERN(LogFabulousDualSense, Log, All)

namespace DsUtility
{
	FABULOUSDUALSENSE_API constexpr FStringView ReturnValueToString(DS5W_ReturnValue ReturnValue);

	FABULOUSDUALSENSE_API constexpr FStringView DeviceConnectionToString(DS5W::DeviceConnection DeviceConnection);
}
