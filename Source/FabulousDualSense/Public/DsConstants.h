#pragma once

#include "InputCoreTypes.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"

namespace DsConstants
{
	inline static const FName InputDeviceName{TEXTVIEW("DsInputDevice")};
	inline static const FString HardwareDeviceIdentifier{TEXTVIEW("DualSense")};

	inline static constexpr auto MaxDevices{4};
	inline static constexpr auto ButtonsCount{29};
	inline static constexpr auto StickDeadZone{30};
	inline static constexpr auto TriggerDeadZone{30};

	inline static const FKey TouchpadKey{FName{TEXTVIEW("DsTouchpad")}};
	inline static const FKey LogoKey{FName{TEXTVIEW("DsLogo")}};
	inline static const FKey MuteKey{FName{TEXTVIEW("DsMute")}};

	inline static const FKey Touch1Key{FName{TEXTVIEW("DsTouch1")}};
	inline static const FKey Touch1AxisXKey{FName{TEXTVIEW("DsTouch1AxisX")}};
	inline static const FKey Touch1AxisYKey{FName{TEXTVIEW("DsTouch1AxisY")}};
	inline static const FKey Touch1AxisXYKey{FName{TEXTVIEW("DsTouch1AxisXY")}};

	inline static const FKey Touch2Key{FName{TEXTVIEW("DsTouch2")}};
	inline static const FKey Touch2AxisXKey{FName{TEXTVIEW("DsTouch2AxisX")}};
	inline static const FKey Touch2AxisYKey{FName{TEXTVIEW("DsTouch2AxisY")}};
	inline static const FKey Touch2AxisXYKey{FName{TEXTVIEW("DsTouch2AxisXY")}};

	FABULOUSDUALSENSE_API const TMap<FGamepadKeyNames::Type, uint32>& GetRegularButtons();
}
