#include "DsUtility.h"

DEFINE_LOG_CATEGORY(LogFabulousDualSense)

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
