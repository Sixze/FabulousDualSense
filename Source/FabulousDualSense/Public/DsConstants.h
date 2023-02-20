#pragma once

#include "InputCoreTypes.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"

namespace DsConstants
{
	inline static constexpr auto MaxDevices{4};
	inline static constexpr auto ButtonsCount{29};
	inline static constexpr auto StickDeadZone{30};
	inline static constexpr auto TriggerDeadZone{30};

	inline static const FKey TouchpadKey{TEXT("DsTouchpad")};
	inline static const FKey LogoKey{TEXT("DsLogo")};
	inline static const FKey MuteKey{TEXT("DsMute")};

	inline static const FKey Touch1Key{TEXT("DsTouch1")};
	inline static const FKey Touch1AxisXKey{TEXT("DsTouch1AxisX")};
	inline static const FKey Touch1AxisYKey{TEXT("DsTouch1AxisY")};
	inline static const FKey Touch1AxisXYKey{TEXT("DsTouch1AxisXY")};

	inline static const FKey Touch2Key{TEXT("DsTouch2")};
	inline static const FKey Touch2AxisXKey{TEXT("DsTouch2AxisX")};
	inline static const FKey Touch2AxisYKey{TEXT("DsTouch2AxisY")};
	inline static const FKey Touch2AxisXYKey{TEXT("DsTouch2AxisXY")};

	FABULOUSDUALSENSE_API const TMap<FGamepadKeyNames::Type, uint32>& GetRegularButtons();
}
