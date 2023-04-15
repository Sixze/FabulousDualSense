#pragma once

#include "InputCoreTypes.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"

namespace DsConstants
{
	FABULOUSDUALSENSE_API extern const FName InputDeviceName;
	FABULOUSDUALSENSE_API extern const FString HardwareDeviceIdentifier;

	inline constexpr auto MaxDevicesCount{4};
	inline constexpr auto ButtonsCount{29};
	inline constexpr auto StickDeadZone{30};
	inline constexpr auto TriggerDeadZone{30};

	FABULOUSDUALSENSE_API extern const FKey TouchpadKey;
	FABULOUSDUALSENSE_API extern const FKey LogoKey;
	FABULOUSDUALSENSE_API extern const FKey MuteKey;

	FABULOUSDUALSENSE_API extern const FKey Touch1Key;
	FABULOUSDUALSENSE_API extern const FKey Touch1AxisXKey;
	FABULOUSDUALSENSE_API extern const FKey Touch1AxisYKey;
	FABULOUSDUALSENSE_API extern const FKey Touch1AxisXYKey;

	FABULOUSDUALSENSE_API extern const FKey Touch2Key;
	FABULOUSDUALSENSE_API extern const FKey Touch2AxisXKey;
	FABULOUSDUALSENSE_API extern const FKey Touch2AxisYKey;
	FABULOUSDUALSENSE_API extern const FKey Touch2AxisXYKey;

	FABULOUSDUALSENSE_API const TMap<FGamepadKeyNames::Type, uint32>& GetRegularButtons();
}
