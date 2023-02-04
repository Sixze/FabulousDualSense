#pragma once

#include "InputCoreTypes.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"

namespace DsConstants
{
	static constexpr auto MaxDevices{4};
	static constexpr auto ButtonsCount{29};
	static constexpr auto StickDeadZone{30};
	static constexpr auto TriggerDeadZone{30};

	static const FKey TouchpadKey{TEXT("DsTouchpad")};
	static const FKey LogoKey{TEXT("DsLogo")};
	static const FKey MuteKey{TEXT("DsMute")};

	static const FKey Touch1Key{TEXT("DsTouch1")};
	static const FKey Touch1AxisXKey{TEXT("DsTouch1AxisX")};
	static const FKey Touch1AxisYKey{TEXT("DsTouch1AxisY")};
	static const FKey Touch1AxisXYKey{TEXT("DsTouch1AxisXY")};

	static const FKey Touch2Key{TEXT("DsTouch2")};
	static const FKey Touch2AxisXKey{TEXT("DsTouch2AxisX")};
	static const FKey Touch2AxisYKey{TEXT("DsTouch2AxisY")};
	static const FKey Touch2AxisXYKey{TEXT("DsTouch2AxisXY")};

	FABULOUSDUALSENSE_API const TMap<FGamepadKeyNames::Type, uint32>& GetRegularButtons();
}
