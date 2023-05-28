#include "DsConstants.h"

#include <DualSenseWindows.h>

const FName DsConstants::InputDeviceName{TEXTVIEW("DsInputDevice")};
const FString DsConstants::HardwareDeviceIdentifier{TEXTVIEW("DualSense")};

const FKey DsConstants::TouchpadKey{FName{TEXTVIEW("DsTouchpad")}};
const FKey DsConstants::LogoKey{FName{TEXTVIEW("DsLogo")}};
const FKey DsConstants::MuteKey{FName{TEXTVIEW("DsMute")}};

const FKey DsConstants::Touch1Key{FName{TEXTVIEW("DsTouch1")}};
const FKey DsConstants::Touch1AxisXKey{FName{TEXTVIEW("DsTouch1AxisX")}};
const FKey DsConstants::Touch1AxisYKey{FName{TEXTVIEW("DsTouch1AxisY")}};
const FKey DsConstants::Touch1AxisXYKey{FName{TEXTVIEW("DsTouch1AxisXY")}};

const FKey DsConstants::Touch2Key{FName{TEXTVIEW("DsTouch2")}};
const FKey DsConstants::Touch2AxisXKey{FName{TEXTVIEW("DsTouch2AxisX")}};
const FKey DsConstants::Touch2AxisYKey{FName{TEXTVIEW("DsTouch2AxisY")}};
const FKey DsConstants::Touch2AxisXYKey{FName{TEXTVIEW("DsTouch2AxisXY")}};

const TMap<FGamepadKeyNames::Type, uint32>& DsConstants::GetRegularButtons()
{
	static const TMap<FGamepadKeyNames::Type, uint32> Buttons{
		{FGamepadKeyNames::DPadUp, DS5W_ISTATE_BTN_DPAD_UP},
		{FGamepadKeyNames::DPadDown, DS5W_ISTATE_BTN_DPAD_DOWN},
		{FGamepadKeyNames::DPadLeft, DS5W_ISTATE_BTN_DPAD_LEFT},
		{FGamepadKeyNames::DPadRight, DS5W_ISTATE_BTN_DPAD_RIGHT},

		{FGamepadKeyNames::FaceButtonTop, DS5W_ISTATE_BTN_TRIANGLE},
		{FGamepadKeyNames::FaceButtonBottom, DS5W_ISTATE_BTN_CROSS},
		{FGamepadKeyNames::FaceButtonLeft, DS5W_ISTATE_BTN_SQUARE},
		{FGamepadKeyNames::FaceButtonRight, DS5W_ISTATE_BTN_CIRCLE},

		{FGamepadKeyNames::LeftTriggerThreshold, DS5W_ISTATE_BTN_TRIGGER_LEFT},
		{FGamepadKeyNames::RightTriggerThreshold, DS5W_ISTATE_BTN_TRIGGER_RIGHT},

		{FGamepadKeyNames::LeftShoulder, DS5W_ISTATE_BTN_BUMPER_LEFT},
		{FGamepadKeyNames::RightShoulder, DS5W_ISTATE_BTN_BUMPER_RIGHT},

		{FGamepadKeyNames::LeftThumb, DS5W_ISTATE_BTN_STICK_LEFT},
		{FGamepadKeyNames::RightThumb, DS5W_ISTATE_BTN_STICK_RIGHT},

		{FGamepadKeyNames::SpecialLeft, DS5W_ISTATE_BTN_SELECT},
		{FGamepadKeyNames::SpecialRight, DS5W_ISTATE_BTN_MENU},

		{TouchpadKey.GetFName(), DS5W_ISTATE_BTN_PAD_BUTTON},
		{LogoKey.GetFName(), DS5W_ISTATE_BTN_PLAYSTATION_LOGO},
		{MuteKey.GetFName(), DS5W_ISTATE_BTN_MIC_BUTTON},
	};

	return Buttons;
}
