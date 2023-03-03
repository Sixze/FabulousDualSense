#include "DsUtility.h"

DEFINE_LOG_CATEGORY(LogFabulousDualSense)

constexpr FStringView DsUtility::ReturnValueToString(const DS5W_ReturnValue ReturnValue)
{
	if (ReturnValue < DS5W_ReturnValue::OK || ReturnValue > DS5W_ReturnValue::E_IO_PENDING)
	{
		return TEXTVIEW("Unknown return value");
	}

	static constexpr FStringView Strings[]
	{
		TEXTVIEW("Operation completed without an error"),
		TEXTVIEW("Operation encountered an unknown error"),
		TEXTVIEW("The user supplied buffer is too small"),
		TEXTVIEW("External unexpected WinAPI error"),
		TEXTVIEW("Not enough memory on the stack"),
		TEXTVIEW("Invalid arguments"),
		TEXTVIEW("This feature is currently not supported"),
		TEXTVIEW("Device was disconnected"),
		TEXTVIEW("Bluetooth communication error"),
		TEXTVIEW("IO timeout"),
		TEXTVIEW("IO failed"),
		TEXTVIEW("Overlapped IO request was not found"),
		TEXTVIEW("IO did not complete because it is running in the background"),
	};

	return Strings[static_cast<uint32>(ReturnValue)];
}

constexpr FStringView DsUtility::DeviceConnectionToString(const DS5W::DeviceConnection DeviceConnection)
{
	if (DeviceConnection < DS5W::DeviceConnection::USB || DeviceConnection > DS5W::DeviceConnection::BT)
	{
		return TEXTVIEW("Unknown device connection");
	}

	static constexpr FStringView Strings[]
	{
		TEXTVIEW("USB"),
		TEXTVIEW("Bluetooth")
	};

	return Strings[static_cast<uint8>(DeviceConnection)];
}
