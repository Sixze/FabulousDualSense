#include "DsConstants.h"

#include <DualSenseWindows.h>

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
