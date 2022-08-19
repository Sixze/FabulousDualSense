#include "DsInputDevice.h"

#include "DsSettings.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/IInputInterface.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CoreDelegates.h"

FDsInputDevice::FDsInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) : MessageHandler{InMessageHandler}
{
	Settings = GetDefault<UDsSettings>();

	GConfig->GetFloat(TEXT("/Script/Engine.InputSettings"), TEXT("InitialButtonRepeatDelay"), InitialButtonRepeatDelay, GInputIni);
	GConfig->GetFloat(TEXT("/Script/Engine.InputSettings"), TEXT("ButtonRepeatDelay"), ButtonRepeatDelay, GInputIni);

	RefreshDevices();
}

FDsInputDevice::~FDsInputDevice()
{
	for (auto ControllerId{0}; ControllerId < DsConstants::MaxDevices; ControllerId++)
	{
		if (DeviceContexts[ControllerId]._internal.connected)
		{
			DisconnectDevice(ControllerId);
		}
	}
}

void FDsInputDevice::Tick(float DeltaTime)
{
	RefreshDevices();
}

void FDsInputDevice::SendControllerEvents()
{
	const auto Time{FPlatformTime::Seconds()};

	for (auto ControllerId{0}; ControllerId < DsConstants::MaxDevices; ControllerId++)
	{
		if (!DeviceContexts[ControllerId]._internal.connected)
		{
			continue;
		}

		// ReSharper disable once CppLocalVariableWithNonTrivialDtorIsNeverUsed
		FInputDeviceScope InputScope{this, DS_GET_TYPE_STRING(FDsInputDevice), ControllerId, TEXT("DualSense")};

		const auto PreviousInput{InputStates[ControllerId]};

		auto& Context{DeviceContexts[ControllerId]};
		auto& Input{InputStates[ControllerId]};
		auto& Output{OutputStates[ControllerId]};

		const auto ReadInputResult{getDeviceInputState(&Context, &Input)};
		if (DS5W_FAILED(ReadInputResult))
		{
			UE_LOG(LogFabulousDualSense, Warning, TEXT("Failed to read device input state: %s, Device: %s."),
			       DsUtility::ReturnValueToString(ReadInputResult), Context._internal.devicePath);

			DisconnectDevice(ControllerId);
			continue;
		}

		// Sticks.

		ProcessStick(ControllerId, FGamepadKeyNames::LeftAnalogX, PreviousInput.leftStick.x, Input.leftStick.x);
		ProcessStick(ControllerId, FGamepadKeyNames::LeftAnalogY, PreviousInput.leftStick.y, Input.leftStick.y);

		ProcessStick(ControllerId, FGamepadKeyNames::RightAnalogX, PreviousInput.rightStick.x, Input.rightStick.x);
		ProcessStick(ControllerId, FGamepadKeyNames::RightAnalogY, PreviousInput.rightStick.y, Input.rightStick.y);

		// Triggers.

		if (PreviousInput.leftTrigger != Input.leftTrigger)
		{
			MessageHandler->OnControllerAnalog(FGamepadKeyNames::LeftTriggerAnalog, ControllerId,
			                                   Input.leftTrigger / static_cast<float>(TNumericLimits<uint8>::Max()));
		}

		if (PreviousInput.rightTrigger != Input.rightTrigger)
		{
			MessageHandler->OnControllerAnalog(FGamepadKeyNames::RightTriggerAnalog, ControllerId,
			                                   Input.rightTrigger / static_cast<float>(TNumericLimits<uint8>::Max()));
		}

		// Regular buttons.

		auto ButtonIndex{0};

		for (const auto& [ButtonName, ButtonFlag] : DsConstants::GetRegularButtons())
		{
			ProcessButton(ControllerId, ButtonName, ButtonIndex,
			              (PreviousInput.buttonMap & ButtonFlag) > 0,
			              (Input.buttonMap & ButtonFlag) > 0, Time);
			ButtonIndex += 1;
		}

		// Virtual buttons.

		ProcessButton(ControllerId, FGamepadKeyNames::LeftStickUp, ButtonIndex,
		              PreviousInput.leftStick.y > DsConstants::VirtualStickButtonDeadZone,
		              Input.leftStick.y > DsConstants::VirtualStickButtonDeadZone, Time);
		ButtonIndex += 1;

		ProcessButton(ControllerId, FGamepadKeyNames::LeftStickDown, ButtonIndex,
		              PreviousInput.leftStick.y < -DsConstants::VirtualStickButtonDeadZone,
		              Input.leftStick.y < -DsConstants::VirtualStickButtonDeadZone, Time);
		ButtonIndex += 1;

		ProcessButton(ControllerId, FGamepadKeyNames::LeftStickLeft, ButtonIndex,
		              PreviousInput.leftStick.x < -DsConstants::VirtualStickButtonDeadZone,
		              Input.leftStick.x < -DsConstants::VirtualStickButtonDeadZone, Time);
		ButtonIndex += 1;

		ProcessButton(ControllerId, FGamepadKeyNames::LeftStickRight, ButtonIndex,
		              PreviousInput.leftStick.x > DsConstants::VirtualStickButtonDeadZone,
		              Input.leftStick.x > DsConstants::VirtualStickButtonDeadZone, Time);
		ButtonIndex += 1;

		ProcessButton(ControllerId, FGamepadKeyNames::RightStickUp, ButtonIndex,
		              PreviousInput.rightStick.y > DsConstants::VirtualStickButtonDeadZone,
		              Input.rightStick.y > DsConstants::VirtualStickButtonDeadZone, Time);
		ButtonIndex += 1;

		ProcessButton(ControllerId, FGamepadKeyNames::RightStickDown, ButtonIndex,
		              PreviousInput.rightStick.y < -DsConstants::VirtualStickButtonDeadZone,
		              Input.rightStick.y < -DsConstants::VirtualStickButtonDeadZone, Time);
		ButtonIndex += 1;

		ProcessButton(ControllerId, FGamepadKeyNames::RightStickLeft, ButtonIndex,
		              PreviousInput.rightStick.x < -DsConstants::VirtualStickButtonDeadZone,
		              Input.rightStick.x < -DsConstants::VirtualStickButtonDeadZone, Time);
		ButtonIndex += 1;

		ProcessButton(ControllerId, FGamepadKeyNames::RightStickRight, ButtonIndex,
		              PreviousInput.rightStick.x > DsConstants::VirtualStickButtonDeadZone,
		              Input.rightStick.x > DsConstants::VirtualStickButtonDeadZone, Time);
		ButtonIndex += 1;

		// Touch pad.

		ProcessButton(ControllerId, DsConstants::Touch1Key.GetFName(), ButtonIndex,
		              PreviousInput.touchPoint1.down, Input.touchPoint1.down, Time);
		ButtonIndex += 1;

		ProcessButton(ControllerId, DsConstants::Touch2Key.GetFName(), ButtonIndex,
		              PreviousInput.touchPoint2.down, Input.touchPoint2.down, Time);
		ButtonIndex += 1;

		ProcessTouch(ControllerId, DsConstants::Touch1AxisXKey.GetFName(), DsConstants::Touch1AxisYKey.GetFName(),
		             PreviousInput.touchPoint1, Input.touchPoint1);

		ProcessTouch(ControllerId, DsConstants::Touch2AxisXKey.GetFName(), DsConstants::Touch2AxisYKey.GetFName(),
		             PreviousInput.touchPoint2, Input.touchPoint2);

		if (PreviousInput.touchPoint1.down && Input.touchPoint1.down)
		{
			const auto TouchAxisX{static_cast<int32>(Input.touchPoint1.x) - static_cast<int32>(PreviousInput.touchPoint1.x)};
			const auto TouchAxisY{static_cast<int32>(Input.touchPoint1.y) - static_cast<int32>(PreviousInput.touchPoint1.y)};

			if (Settings->bEmitMouseEventsFromTouchpad)
			{
				MessageHandler->OnRawMouseMove(TouchAxisX, TouchAxisY);
			}
		}

		if (ExtraStates[ControllerId].bOutputChanged)
		{
			const auto WriteOutputResult{setDeviceOutputState(&Context, &Output)};
			if (DS5W_FAILED(WriteOutputResult))
			{
				UE_LOG(LogFabulousDualSense, Warning, TEXT("Failed to write device output state: %s, Device: %s."),
				       DsUtility::ReturnValueToString(ReadInputResult), Context._internal.devicePath);

				DisconnectDevice(ControllerId);
			}
		}
	}
}

void FDsInputDevice::SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	MessageHandler = InMessageHandler;
}

bool FDsInputDevice::Exec(UWorld* World, const TCHAR* Command, FOutputDevice& Archive)
{
	return false;
}

void FDsInputDevice::SetChannelValue(const int32 ControllerId, const FForceFeedbackChannelType ChannelType, const float Value)
{
	if (ControllerId < 0 || ControllerId >= DsConstants::MaxDevices || !DeviceContexts[ControllerId]._internal.connected)
	{
		return;
	}

	auto& Extra{ExtraStates[ControllerId]};

	switch (ChannelType)
	{
		case FForceFeedbackChannelType::LEFT_LARGE:
			Extra.ForceFeedbackLeftLarge = static_cast<uint8>(Value * TNumericLimits<uint8>::Max());
			break;

		case FForceFeedbackChannelType::LEFT_SMALL:
			Extra.ForceFeedbackLeftSmall = static_cast<uint8>(Value * TNumericLimits<uint8>::Max());
			break;

		case FForceFeedbackChannelType::RIGHT_LARGE:
			Extra.ForceFeedbackRightLarge = static_cast<uint8>(Value * TNumericLimits<uint8>::Max());
			break;

		case FForceFeedbackChannelType::RIGHT_SMALL:
			Extra.ForceFeedbackRightSmall = static_cast<uint8>(Value * TNumericLimits<uint8>::Max());
			break;
	}

	const auto NewForceFeedbackLeft{FMath::Max(Extra.ForceFeedbackLeftLarge, Extra.ForceFeedbackLeftSmall)};
	const auto NewForceFeedbackRight{FMath::Max(Extra.ForceFeedbackRightLarge, Extra.ForceFeedbackRightSmall)};

	auto& Output{OutputStates[ControllerId]};

	Extra.bOutputChanged = Output.leftRumble != NewForceFeedbackLeft || Output.rightRumble != NewForceFeedbackRight;

	Output.leftRumble = NewForceFeedbackLeft;
	Output.rightRumble = NewForceFeedbackRight;
}

void FDsInputDevice::SetChannelValues(const int32 ControllerId, const FForceFeedbackValues& Values)
{
	if (ControllerId < 0 || ControllerId >= DsConstants::MaxDevices || !DeviceContexts[ControllerId]._internal.connected)
	{
		return;
	}

	auto& Extra{ExtraStates[ControllerId]};

	Extra.ForceFeedbackLeftLarge = static_cast<uint8>(Values.LeftLarge * TNumericLimits<uint8>::Max());
	Extra.ForceFeedbackLeftSmall = static_cast<uint8>(Values.LeftSmall * TNumericLimits<uint8>::Max());
	Extra.ForceFeedbackRightLarge = static_cast<uint8>(Values.RightLarge * TNumericLimits<uint8>::Max());
	Extra.ForceFeedbackRightSmall = static_cast<uint8>(Values.RightSmall * TNumericLimits<uint8>::Max());

	const auto NewForceFeedbackLeft{FMath::Max(Extra.ForceFeedbackLeftLarge, Extra.ForceFeedbackLeftSmall)};
	const auto NewForceFeedbackRight{FMath::Max(Extra.ForceFeedbackRightLarge, Extra.ForceFeedbackRightSmall)};

	auto& Output{OutputStates[ControllerId]};

	Extra.bOutputChanged = Output.leftRumble != NewForceFeedbackLeft || Output.rightRumble != NewForceFeedbackRight;

	Output.leftRumble = NewForceFeedbackLeft;
	Output.rightRumble = NewForceFeedbackRight;
}

bool FDsInputDevice::IsGamepadAttached() const
{
	auto bResult{false};

	for (auto i{0}; i < DsConstants::MaxDevices; i++)
	{
		bResult |= DeviceContexts[i]._internal.connected;
	}

	return bResult;
}

void FDsInputDevice::RefreshDevices()
{
	DS5W::DeviceEnumInfo DeviceInfos[DsConstants::MaxDevices];
	uint32 DevicesCount{0};

	switch (const auto EnumDevicesResult{enumDevices(DeviceInfos, DsConstants::MaxDevices, &DevicesCount)})
	{
		case DS5W_OK:
		case DS5W_E_INSUFFICIENT_BUFFER:
			break;

		default:
			UE_LOG(LogFabulousDualSense, Warning, TEXT("Failed to enumerate devices: %s."),
			       DsUtility::ReturnValueToString(EnumDevicesResult));
			return;
	}

	for (uint32 i{0}; i < DevicesCount; i++)
	{
		auto bAlreadyConnected{false};
		auto FreeIndex{-1};

		for (auto j{0}; j < DsConstants::MaxDevices; j++)
		{
			bAlreadyConnected |= DeviceInfos[i]._internal.uniqueID == DeviceContexts[j]._internal.uniqueID;

			if (FreeIndex < 0 && !DeviceContexts[j]._internal.connected)
			{
				FreeIndex = j;
			}
		}

		if (bAlreadyConnected || FreeIndex < 0)
		{
			continue;
		}

		UE_LOG(LogFabulousDualSense, Log, TEXT("New device found: %s, Connection: %s."),
		       DeviceInfos[i]._internal.path, DsUtility::DeviceConnectionToString(DeviceInfos[i]._internal.connection));

		const auto InitializeDeviceContextResult{initDeviceContext(&DeviceInfos[i], &DeviceContexts[FreeIndex])};
		if (DS5W_SUCCESS(InitializeDeviceContextResult))
		{
			UE_LOG(LogFabulousDualSense, Log, TEXT("Device connected: %s."), DeviceInfos[i]._internal.path);

			FMemory::Memzero(InputStates[FreeIndex]);
			FMemory::Memzero(OutputStates[FreeIndex]);
			FMemory::Memzero(ExtraStates[FreeIndex]);

			FCoreDelegates::OnControllerConnectionChange.Broadcast(true, PLATFORMUSERID_NONE, i);
		}
		else
		{
			UE_LOG(LogFabulousDualSense, Warning, TEXT("Failed to initialize device context: %s, Device: %s."),
			       DsUtility::ReturnValueToString(InitializeDeviceContextResult), DeviceInfos[i]._internal.path);

			FMemory::Memzero(DeviceContexts[FreeIndex]);
		}
	}
}

void FDsInputDevice::DisconnectDevice(const int32 ControllerId)
{
	auto& Context{DeviceContexts[ControllerId]};

	UE_LOG(LogFabulousDualSense, Log, TEXT("Device disconnected: %s."), Context._internal.devicePath);

	freeDeviceContext(&Context);
	Context._internal.uniqueID = 0;

	if (FSlateApplication::Get().GetPlatformApplication().IsValid())
	{
		const auto& Input{InputStates[ControllerId]};

		// Release sticks.

		MessageHandler->OnControllerAnalog(FGamepadKeyNames::LeftAnalogX, ControllerId, 0.0f);
		MessageHandler->OnControllerAnalog(FGamepadKeyNames::LeftAnalogY, ControllerId, 0.0f);

		MessageHandler->OnControllerAnalog(FGamepadKeyNames::RightAnalogX, ControllerId, 0.0f);
		MessageHandler->OnControllerAnalog(FGamepadKeyNames::RightAnalogY, ControllerId, 0.0f);

		// Release triggers.

		MessageHandler->OnControllerAnalog(FGamepadKeyNames::LeftTriggerAnalog, ControllerId, 0.0f);
		MessageHandler->OnControllerAnalog(FGamepadKeyNames::RightTriggerAnalog, ControllerId, 0.0f);

		// Release regular buttons.

		for (const auto& [ButtonName, ButtonFlag] : DsConstants::GetRegularButtons())
		{
			ReleaseButton(ControllerId, ButtonName, (Input.buttonMap & ButtonFlag) > 0);
		}

		// Release virtual buttons.d

		ReleaseButton(ControllerId, FGamepadKeyNames::LeftStickUp, Input.leftStick.y > DsConstants::VirtualStickButtonDeadZone);
		ReleaseButton(ControllerId, FGamepadKeyNames::LeftStickDown, Input.leftStick.y < -DsConstants::VirtualStickButtonDeadZone);
		ReleaseButton(ControllerId, FGamepadKeyNames::LeftStickLeft, Input.leftStick.x < -DsConstants::VirtualStickButtonDeadZone);
		ReleaseButton(ControllerId, FGamepadKeyNames::LeftStickRight, Input.leftStick.x > DsConstants::VirtualStickButtonDeadZone);
		ReleaseButton(ControllerId, FGamepadKeyNames::RightStickUp, Input.rightStick.y > DsConstants::VirtualStickButtonDeadZone);
		ReleaseButton(ControllerId, FGamepadKeyNames::RightStickDown, Input.rightStick.y < -DsConstants::VirtualStickButtonDeadZone);
		ReleaseButton(ControllerId, FGamepadKeyNames::RightStickLeft, Input.rightStick.x < -DsConstants::VirtualStickButtonDeadZone);
		ReleaseButton(ControllerId, FGamepadKeyNames::RightStickRight, Input.rightStick.x > DsConstants::VirtualStickButtonDeadZone);

		// Release touch pad.

		ReleaseButton(ControllerId, DsConstants::Touch1Key.GetFName(), Input.touchPoint1.down);
		ReleaseButton(ControllerId, DsConstants::Touch2Key.GetFName(), Input.touchPoint2.down);
	}

	FCoreDelegates::OnControllerConnectionChange.Broadcast(false, PLATFORMUSERID_NONE, ControllerId);
}

void FDsInputDevice::ProcessStick(const int32 ControllerId, const FGamepadKeyNames::Type& KeyName,
                                  const int8 PreviousValue, const int8 NewValue) const
{
	if (PreviousValue != NewValue)
	{
		const auto Scale{
			NewValue <= 0
				? 1.0f / -static_cast<float>(TNumericLimits<int8>::Min())
				: 1.0f / static_cast<float>(TNumericLimits<int8>::Max())
		};

		MessageHandler->OnControllerAnalog(KeyName, ControllerId, NewValue * Scale);
	}
}

void FDsInputDevice::ProcessButton(const int32 ControllerId, const FGamepadKeyNames::Type& KeyName, const int32 ButtonIndex,
                                   const bool bPreviousKeyDown, const bool bNewKeyDown, const double Time)
{
	if (bPreviousKeyDown != bNewKeyDown)
	{
		if (bNewKeyDown)
		{
			MessageHandler->OnControllerButtonPressed(KeyName, ControllerId, false);

			ExtraStates[ControllerId].ButtonsNextRepeatTime[ButtonIndex] = Time + InitialButtonRepeatDelay;
		}
		else
		{
			MessageHandler->OnControllerButtonReleased(KeyName, ControllerId, false);
		}

		return;
	}

	if (bNewKeyDown && ExtraStates[ControllerId].ButtonsNextRepeatTime[ButtonIndex] <= Time)
	{
		MessageHandler->OnControllerButtonPressed(KeyName, ControllerId, true);

		ExtraStates[ControllerId].ButtonsNextRepeatTime[ButtonIndex] = Time + ButtonRepeatDelay;
	}
}

void FDsInputDevice::ProcessTouch(const int32 ControllerId, const FGamepadKeyNames::Type& AxisXKeyName,
                                  const FGamepadKeyNames::Type& AxisYKeyName,
                                  const DS5W::Touch& PreviousTouch, const DS5W::Touch& NewTouch) const
{
	if (!PreviousTouch.down || !NewTouch.down)
	{
		return;
	}

	const auto TouchAxisX{static_cast<int32>(NewTouch.x) - static_cast<int32>(PreviousTouch.x)};
	if (TouchAxisX != 0)
	{
		MessageHandler->OnControllerAnalog(AxisXKeyName, ControllerId, TouchAxisX);
	}

	const auto TouchAxisY{static_cast<int32>(NewTouch.y) - static_cast<int32>(PreviousTouch.y)};
	if (TouchAxisY != 0)
	{
		MessageHandler->OnControllerAnalog(AxisYKeyName, ControllerId, TouchAxisY);
	}

	if (Settings->bEmitMouseEventsFromTouchpad)
	{
		MessageHandler->OnRawMouseMove(TouchAxisX, TouchAxisY);
	}
}

void FDsInputDevice::ReleaseButton(const int32 ControllerId, const FGamepadKeyNames::Type& KeyName, const bool bPressed) const
{
	if (bPressed)
	{
		MessageHandler->OnControllerButtonReleased(KeyName, ControllerId, false);
	}
}
