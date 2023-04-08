#include "DsInputDevice.h"

#include "DsSettings.h"
#include "DsUtility.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/IInputInterface.h"
#include "Misc/ConfigCacheIni.h"

FDsInputDevice::FDsInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& MessageHandler) : MessageHandler{MessageHandler}
{
	Settings = GetDefault<UDsSettings>();

	GConfig->GetFloat(TEXT("/Script/Engine.InputSettings"), TEXT("InitialButtonRepeatDelay"), InitialButtonRepeatDelay, GInputIni);
	GConfig->GetFloat(TEXT("/Script/Engine.InputSettings"), TEXT("ButtonRepeatDelay"), ButtonRepeatDelay, GInputIni);

	RefreshDevices();
}

FDsInputDevice::~FDsInputDevice()
{
	auto& InputDeviceMapper{IPlatformInputDeviceMapper::Get()};

	for (auto ControllerId{0}; ControllerId < DsConstants::MaxDevices; ControllerId++)
	{
		if (DeviceContexts[ControllerId]._internal.connected)
		{
			auto PlatformUserId{PLATFORMUSERID_NONE};
			auto InputDeviceId{INPUTDEVICEID_NONE};
			InputDeviceMapper.RemapControllerIdToPlatformUserAndDevice(ControllerId, PlatformUserId, InputDeviceId);

			DisconnectDevice(InputDeviceMapper, ControllerId, PlatformUserId, InputDeviceId);
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

	auto& InputDeviceMapper{IPlatformInputDeviceMapper::Get()};

	for (auto ControllerId{0}; ControllerId < DsConstants::MaxDevices; ControllerId++)
	{
		if (!DeviceContexts[ControllerId]._internal.connected)
		{
			continue;
		}

		FInputDeviceScope InputDeviceScope{this, DsConstants::InputDeviceName, ControllerId, DsConstants::HardwareDeviceIdentifier};

		auto PlatformUserId{PLATFORMUSERID_NONE};
		auto InputDeviceId{INPUTDEVICEID_NONE};
		InputDeviceMapper.RemapControllerIdToPlatformUserAndDevice(ControllerId, PlatformUserId, InputDeviceId);

		const auto PreviousInput{InputStates[ControllerId]};

		auto& Context{DeviceContexts[ControllerId]};
		auto& Input{InputStates[ControllerId]};
		auto& Output{OutputStates[ControllerId]};

		const auto ReadInputResult{getDeviceInputState(&Context, &Input)};
		if (DS5W_FAILED(ReadInputResult))
		{
			UE_LOG(LogFabulousDualSense, Warning, TEXT("Failed to read device input state: %s, Device: %s."),
			       DsUtility::ReturnValueToString(ReadInputResult).GetData(), Context._internal.devicePath);

			DisconnectDevice(InputDeviceMapper, ControllerId, PlatformUserId, InputDeviceId);
			continue;
		}

		// Sticks.

		ProcessStick(PlatformUserId, InputDeviceId, FGamepadKeyNames::LeftAnalogX, PreviousInput.leftStick.x, Input.leftStick.x);
		ProcessStick(PlatformUserId, InputDeviceId, FGamepadKeyNames::LeftAnalogY, PreviousInput.leftStick.y, Input.leftStick.y);

		ProcessStick(PlatformUserId, InputDeviceId, FGamepadKeyNames::RightAnalogX, PreviousInput.rightStick.x, Input.rightStick.x);
		ProcessStick(PlatformUserId, InputDeviceId, FGamepadKeyNames::RightAnalogY, PreviousInput.rightStick.y, Input.rightStick.y);

		// Triggers.

		if (PreviousInput.leftTrigger != Input.leftTrigger || Input.leftTrigger > DsConstants::TriggerDeadZone)
		{
			MessageHandler->OnControllerAnalog(FGamepadKeyNames::LeftTriggerAnalog, PlatformUserId, InputDeviceId,
			                                   Input.leftTrigger / static_cast<float>(TNumericLimits<uint8>::Max()));
		}

		if (PreviousInput.rightTrigger != Input.rightTrigger || Input.rightTrigger > DsConstants::TriggerDeadZone)
		{
			MessageHandler->OnControllerAnalog(FGamepadKeyNames::RightTriggerAnalog, PlatformUserId, InputDeviceId,
			                                   Input.rightTrigger / static_cast<float>(TNumericLimits<uint8>::Max()));
		}

		// Regular buttons.

		auto ButtonIndex{0};

		for (const auto& [ButtonName, ButtonFlag] : DsConstants::GetRegularButtons())
		{
			ProcessButton(ControllerId, PlatformUserId, InputDeviceId, ButtonName, ButtonIndex,
			              (PreviousInput.buttonMap & ButtonFlag) > 0,
			              (Input.buttonMap & ButtonFlag) > 0, Time);
			ButtonIndex += 1;
		}

		// Virtual buttons.

		ProcessButton(ControllerId, PlatformUserId, InputDeviceId, FGamepadKeyNames::LeftStickUp, ButtonIndex,
		              PreviousInput.leftStick.y > DsConstants::StickDeadZone, Input.leftStick.y > DsConstants::StickDeadZone, Time);
		ButtonIndex += 1;

		ProcessButton(ControllerId, PlatformUserId, InputDeviceId, FGamepadKeyNames::LeftStickDown, ButtonIndex,
		              PreviousInput.leftStick.y < -DsConstants::StickDeadZone, Input.leftStick.y < -DsConstants::StickDeadZone, Time);
		ButtonIndex += 1;

		ProcessButton(ControllerId, PlatformUserId, InputDeviceId, FGamepadKeyNames::LeftStickLeft, ButtonIndex,
		              PreviousInput.leftStick.x < -DsConstants::StickDeadZone, Input.leftStick.x < -DsConstants::StickDeadZone, Time);
		ButtonIndex += 1;

		ProcessButton(ControllerId, PlatformUserId, InputDeviceId, FGamepadKeyNames::LeftStickRight, ButtonIndex,
		              PreviousInput.leftStick.x > DsConstants::StickDeadZone, Input.leftStick.x > DsConstants::StickDeadZone, Time);
		ButtonIndex += 1;

		ProcessButton(ControllerId, PlatformUserId, InputDeviceId, FGamepadKeyNames::RightStickUp, ButtonIndex,
		              PreviousInput.rightStick.y > DsConstants::StickDeadZone, Input.rightStick.y > DsConstants::StickDeadZone, Time);
		ButtonIndex += 1;

		ProcessButton(ControllerId, PlatformUserId, InputDeviceId, FGamepadKeyNames::RightStickDown, ButtonIndex,
		              PreviousInput.rightStick.y < -DsConstants::StickDeadZone, Input.rightStick.y < -DsConstants::StickDeadZone, Time);
		ButtonIndex += 1;

		ProcessButton(ControllerId, PlatformUserId, InputDeviceId, FGamepadKeyNames::RightStickLeft, ButtonIndex,
		              PreviousInput.rightStick.x < -DsConstants::StickDeadZone, Input.rightStick.x < -DsConstants::StickDeadZone, Time);
		ButtonIndex += 1;

		ProcessButton(ControllerId, PlatformUserId, InputDeviceId, FGamepadKeyNames::RightStickRight, ButtonIndex,
		              PreviousInput.rightStick.x > DsConstants::StickDeadZone, Input.rightStick.x > DsConstants::StickDeadZone, Time);
		ButtonIndex += 1;

		// Touch pad.

		ProcessButton(ControllerId, PlatformUserId, InputDeviceId, DsConstants::Touch1Key.GetFName(), ButtonIndex,
		              PreviousInput.touchPoint1.down, Input.touchPoint1.down, Time);
		ButtonIndex += 1;

		ProcessButton(ControllerId, PlatformUserId, InputDeviceId, DsConstants::Touch2Key.GetFName(), ButtonIndex,
		              PreviousInput.touchPoint2.down, Input.touchPoint2.down, Time);
		ButtonIndex += 1;

		ProcessTouch(PlatformUserId, InputDeviceId, DsConstants::Touch1AxisXKey.GetFName(),
		             DsConstants::Touch1AxisYKey.GetFName(), PreviousInput.touchPoint1, Input.touchPoint1);

		ProcessTouch(PlatformUserId, InputDeviceId, DsConstants::Touch2AxisXKey.GetFName(),
		             DsConstants::Touch2AxisYKey.GetFName(), PreviousInput.touchPoint2, Input.touchPoint2);

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
				       DsUtility::ReturnValueToString(ReadInputResult).GetData(), Context._internal.devicePath);

				DisconnectDevice(InputDeviceMapper, ControllerId, PlatformUserId, InputDeviceId);
			}
		}
	}
}

void FDsInputDevice::SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& NewMessageHandler)
{
	MessageHandler = NewMessageHandler;
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
	static DS5W::DeviceEnumInfo DeviceInfos[DsConstants::MaxDevices];
	uint32 DevicesCount{0};

	switch (const auto EnumDevicesResult{enumDevices(DeviceInfos, DsConstants::MaxDevices, &DevicesCount)})
	{
		case DS5W_OK:
		case DS5W_E_INSUFFICIENT_BUFFER:
			break;

		default:
			UE_LOG(LogFabulousDualSense, Warning, TEXT("Failed to enumerate devices: %s."),
			       DsUtility::ReturnValueToString(EnumDevicesResult).GetData());
			return;
	}

	auto& InputDeviceMapper{IPlatformInputDeviceMapper::Get()};

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
		       DeviceInfos[i]._internal.path, DsUtility::DeviceConnectionToString(DeviceInfos[i]._internal.connection).GetData());

		const auto InitializeDeviceContextResult{initDeviceContext(&DeviceInfos[i], &DeviceContexts[FreeIndex])};
		if (DS5W_SUCCESS(InitializeDeviceContextResult))
		{
			UE_LOG(LogFabulousDualSense, Log, TEXT("Device connected: %s."), DeviceInfos[i]._internal.path);

			FMemory::Memzero(InputStates[FreeIndex]);
			FMemory::Memzero(OutputStates[FreeIndex]);
			FMemory::Memzero(ExtraStates[FreeIndex]);

			auto PlatformUserId{PLATFORMUSERID_NONE};
			auto InputDeviceId{INPUTDEVICEID_NONE};
			InputDeviceMapper.RemapControllerIdToPlatformUserAndDevice(i, PlatformUserId, InputDeviceId);

			InputDeviceMapper.Internal_MapInputDeviceToUser(InputDeviceId, PlatformUserId, EInputDeviceConnectionState::Connected);
		}
		else
		{
			UE_LOG(LogFabulousDualSense, Warning, TEXT("Failed to initialize device context: %s, Device: %s."),
			       DsUtility::ReturnValueToString(InitializeDeviceContextResult).GetData(), DeviceInfos[i]._internal.path);

			FMemory::Memzero(DeviceContexts[FreeIndex]);
		}
	}
}

void FDsInputDevice::DisconnectDevice(IPlatformInputDeviceMapper& InputDeviceMapper, const int32 ControllerId,
                                      const FPlatformUserId PlatformUserId, const FInputDeviceId InputDeviceId)
{
	auto& Context{DeviceContexts[ControllerId]};

	UE_LOG(LogFabulousDualSense, Log, TEXT("Device disconnected: %s."), Context._internal.devicePath);

	freeDeviceContext(&Context);
	Context._internal.uniqueID = 0;

	if (FSlateApplication::Get().GetPlatformApplication().IsValid())
	{
		const auto& Input{InputStates[ControllerId]};

		// Release sticks.

		ReleaseStick(PlatformUserId, InputDeviceId, FGamepadKeyNames::LeftAnalogX, Input.leftStick.x);
		ReleaseStick(PlatformUserId, InputDeviceId, FGamepadKeyNames::LeftAnalogY, Input.leftStick.y);

		ReleaseStick(PlatformUserId, InputDeviceId, FGamepadKeyNames::RightAnalogX, Input.rightStick.x);
		ReleaseStick(PlatformUserId, InputDeviceId, FGamepadKeyNames::RightAnalogY, Input.rightStick.y);

		// Release triggers.

		if (Input.leftTrigger != 0)
		{
			MessageHandler->OnControllerAnalog(FGamepadKeyNames::LeftTriggerAnalog, PlatformUserId, InputDeviceId, 0.0f);
		}

		if (Input.rightTrigger != 0)
		{
			MessageHandler->OnControllerAnalog(FGamepadKeyNames::RightTriggerAnalog, PlatformUserId, InputDeviceId, 0.0f);
		}

		// Release regular buttons.

		for (const auto& [ButtonName, ButtonFlag] : DsConstants::GetRegularButtons())
		{
			ReleaseButton(PlatformUserId, InputDeviceId, ButtonName, (Input.buttonMap & ButtonFlag) > 0);
		}

		// Release virtual buttons.

		ReleaseButton(PlatformUserId, InputDeviceId, FGamepadKeyNames::LeftStickUp, Input.leftStick.y > DsConstants::StickDeadZone);
		ReleaseButton(PlatformUserId, InputDeviceId, FGamepadKeyNames::LeftStickDown, Input.leftStick.y < -DsConstants::StickDeadZone);
		ReleaseButton(PlatformUserId, InputDeviceId, FGamepadKeyNames::LeftStickLeft, Input.leftStick.x < -DsConstants::StickDeadZone);
		ReleaseButton(PlatformUserId, InputDeviceId, FGamepadKeyNames::LeftStickRight, Input.leftStick.x > DsConstants::StickDeadZone);
		ReleaseButton(PlatformUserId, InputDeviceId, FGamepadKeyNames::RightStickUp, Input.rightStick.y > DsConstants::StickDeadZone);
		ReleaseButton(PlatformUserId, InputDeviceId, FGamepadKeyNames::RightStickDown, Input.rightStick.y < -DsConstants::StickDeadZone);
		ReleaseButton(PlatformUserId, InputDeviceId, FGamepadKeyNames::RightStickLeft, Input.rightStick.x < -DsConstants::StickDeadZone);
		ReleaseButton(PlatformUserId, InputDeviceId, FGamepadKeyNames::RightStickRight, Input.rightStick.x > DsConstants::StickDeadZone);

		// Release touch pad.

		ReleaseButton(PlatformUserId, InputDeviceId, DsConstants::Touch1Key.GetFName(), Input.touchPoint1.down);
		ReleaseButton(PlatformUserId, InputDeviceId, DsConstants::Touch2Key.GetFName(), Input.touchPoint2.down);
	}

	InputDeviceMapper.Internal_MapInputDeviceToUser(InputDeviceId, PlatformUserId, EInputDeviceConnectionState::Disconnected);
}

void FDsInputDevice::ProcessStick(const FPlatformUserId PlatformUserId, const FInputDeviceId InputDeviceId,
                                  const FGamepadKeyNames::Type& KeyName, const int8 PreviousValue, const int8 NewValue) const
{
	if (PreviousValue != NewValue || FMath::Abs(NewValue) > DsConstants::StickDeadZone)
	{
		const auto Scale{
			NewValue <= 0
				? 1.0f / -static_cast<float>(TNumericLimits<int8>::Min())
				: 1.0f / static_cast<float>(TNumericLimits<int8>::Max())
		};

		MessageHandler->OnControllerAnalog(KeyName, PlatformUserId, InputDeviceId, NewValue * Scale);
	}
}

void FDsInputDevice::ProcessButton(const int32 ControllerId, const FPlatformUserId PlatformUserId, const FInputDeviceId InputDeviceId,
                                   const FGamepadKeyNames::Type& KeyName, const int32 ButtonIndex,
                                   const bool bPreviousKeyDown, const bool bNewKeyDown, const double Time)
{
	if (bPreviousKeyDown != bNewKeyDown)
	{
		if (bNewKeyDown)
		{
			MessageHandler->OnControllerButtonPressed(KeyName, PlatformUserId, InputDeviceId, false);

			ExtraStates[ControllerId].ButtonsNextRepeatTime[ButtonIndex] = Time + InitialButtonRepeatDelay;
		}
		else
		{
			MessageHandler->OnControllerButtonReleased(KeyName, PlatformUserId, InputDeviceId, false);
		}

		return;
	}

	if (bNewKeyDown && ExtraStates[ControllerId].ButtonsNextRepeatTime[ButtonIndex] <= Time)
	{
		MessageHandler->OnControllerButtonPressed(KeyName, PlatformUserId, InputDeviceId, true);

		ExtraStates[ControllerId].ButtonsNextRepeatTime[ButtonIndex] = Time + ButtonRepeatDelay;
	}
}

void FDsInputDevice::ProcessTouch(const FPlatformUserId PlatformUserId, const FInputDeviceId InputDeviceId,
                                  const FGamepadKeyNames::Type& AxisXKeyName, const FGamepadKeyNames::Type& AxisYKeyName,
                                  const DS5W::Touch& PreviousTouch, const DS5W::Touch& NewTouch) const
{
	if (!PreviousTouch.down || !NewTouch.down)
	{
		return;
	}

	const auto TouchAxisX{static_cast<int32>(NewTouch.x) - static_cast<int32>(PreviousTouch.x)};
	if (TouchAxisX != 0)
	{
		MessageHandler->OnControllerAnalog(AxisXKeyName, PlatformUserId, InputDeviceId, TouchAxisX);
	}

	const auto TouchAxisY{static_cast<int32>(NewTouch.y) - static_cast<int32>(PreviousTouch.y)};
	if (TouchAxisY != 0)
	{
		MessageHandler->OnControllerAnalog(AxisYKeyName, PlatformUserId, InputDeviceId, TouchAxisY);
	}

	if (Settings->bEmitMouseEventsFromTouchpad)
	{
		MessageHandler->OnRawMouseMove(TouchAxisX, TouchAxisY);
	}
}

void FDsInputDevice::ReleaseStick(const FPlatformUserId PlatformUserId, const FInputDeviceId InputDeviceId,
                                  const FGamepadKeyNames::Type& KeyName, const int8 CurrentValue) const
{
	if (CurrentValue != 0)
	{
		MessageHandler->OnControllerAnalog(KeyName, PlatformUserId, InputDeviceId, 0.0f);
	}
}

void FDsInputDevice::ReleaseButton(const FPlatformUserId PlatformUserId, const FInputDeviceId InputDeviceId,
                                   const FGamepadKeyNames::Type& KeyName, const bool bPressed) const
{
	if (bPressed)
	{
		MessageHandler->OnControllerButtonReleased(KeyName, PlatformUserId, InputDeviceId, false);
	}
}
