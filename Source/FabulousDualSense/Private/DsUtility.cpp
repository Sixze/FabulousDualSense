#include "DsUtility.h"

DEFINE_LOG_CATEGORY(LogFabulousDualSense)

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

constexpr const TCHAR* DsUtility::ReturnValueToString(const DS5W_ReturnValue ReturnValue)
{
	if (ReturnValue < DS5W_ReturnValue::OK || ReturnValue > DS5W_ReturnValue::E_IO_PENDING)
	{
		return TEXT("Unknown return value");
	}

	static const TCHAR* Strings[]
	{
		TEXT("Operation completed without an error"),
		TEXT("Operation encountered an unknown error"),
		TEXT("The user supplied buffer is too small"),
		TEXT("External unexpected WinAPI error"),
		TEXT("Not enough memory on the stack"),
		TEXT("Invalid arguments"),
		TEXT("This feature is currently not supported"),
		TEXT("Device was disconnected"),
		TEXT("Bluetooth communication error"),
		TEXT("IO timeout"),
		TEXT("IO failed"),
		TEXT("Overlapped IO request was not found"),
		TEXT("IO did not complete because it is running in the background"),
	};

	return Strings[static_cast<uint32>(ReturnValue)];
}

constexpr const TCHAR* DsUtility::DeviceConnectionToString(const DS5W::DeviceConnection DeviceConnection)
{
	if (DeviceConnection < DS5W::DeviceConnection::USB || DeviceConnection > DS5W::DeviceConnection::BT)
	{
		return TEXT("Unknown device connection");
	}

	static const TCHAR* Strings[]
	{
		TEXT("USB"),
		TEXT("Bluetooth")
	};

	return Strings[static_cast<uint8>(DeviceConnection)];
}
