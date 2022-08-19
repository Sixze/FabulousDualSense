#pragma once

#include <DualSenseWindows.h>

#include "InputCoreTypes.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"

FABULOUSDUALSENSE_API DECLARE_LOG_CATEGORY_EXTERN(LogFabulousDualSense, Log, All)

#define DS_GET_TYPE_STRING(Type) \
	((void) sizeof UEAsserts_Private::GetMemberNameCheckedJunk(static_cast<Type*>(nullptr)), TEXT(#Type))

namespace DsConstants
{
	static constexpr auto MaxDevices{4};
	static constexpr auto ButtonsCount{29};
	static constexpr auto VirtualStickButtonDeadZone{30};

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

namespace DsUtility
{
	FABULOUSDUALSENSE_API constexpr const TCHAR* ReturnValueToString(DS5W_ReturnValue ReturnValue);

	FABULOUSDUALSENSE_API constexpr const TCHAR* DeviceConnectionToString(DS5W::DeviceConnection DeviceConnection);
}
