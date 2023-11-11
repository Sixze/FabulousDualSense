#include "DsInputDevice.h"

#include "DsSettings.h"
#include "DsUtility.h"
#include "Containers/StaticBitArray.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/InputSettings.h"
#include "GenericPlatform/IInputInterface.h"
#include "Misc/ConfigCacheIni.h"

FDsInputDevice::FDsInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& MessageHandler) : MessageHandler{MessageHandler}
{
	GConfig->GetFloat(TEXT("/Script/Engine.InputSettings"), TEXT("InitialButtonRepeatDelay"), InitialButtonRepeatDelay, GInputIni);
	GConfig->GetFloat(TEXT("/Script/Engine.InputSettings"), TEXT("ButtonRepeatDelay"), ButtonRepeatDelay, GInputIni);

	RefreshDevices();
}

FDsInputDevice::~FDsInputDevice()
{
	auto& InputDeviceMapper{IPlatformInputDeviceMapper::Get()};

	for (auto ControllerId{0}; ControllerId < DsConstants::MaxDevicesCount; ControllerId++)
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

	for (auto ControllerId{0}; ControllerId < DsConstants::MaxDevicesCount; ControllerId++)
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

		// Gyroscope.

		// Gyroscope X represents Unreal Engine's pitch axis
		if (PreviousInput.gyroscope.x != Input.gyroscope.x)
		{
			MessageHandler->OnControllerAnalog(DsConstants::GyroscopeAxisPitchKey.GetFName(), PlatformUserId, InputDeviceId,
				Input.gyroscope.x * 0.0001);
		}

		// Gyroscope Y represents Unreal Engine's yaw axis
		if (PreviousInput.gyroscope.y != Input.gyroscope.y)
		{
			MessageHandler->OnControllerAnalog(DsConstants::GyroscopeAxisYawKey.GetFName(), PlatformUserId, InputDeviceId,
				Input.gyroscope.y * 0.0001);
		}

		// Gyroscope Z represents Unreal Engine's roll axis
		if (PreviousInput.gyroscope.z != Input.gyroscope.z)
		{
			MessageHandler->OnControllerAnalog(DsConstants::GyroscopeAxisRollKey.GetFName(), PlatformUserId, InputDeviceId,
				Input.gyroscope.z * 0.0001);
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
			const auto TouchAxisX{static_cast<int32>(Input.touchPoint1.x - PreviousInput.touchPoint1.x)};
			const auto TouchAxisY{static_cast<int32>(Input.touchPoint1.y - PreviousInput.touchPoint1.y)};

			if (GetDefault<UDsSettings>()->bEmitMouseEventsFromTouchpad)
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
	if (ControllerId < 0 || ControllerId >= DsConstants::MaxDevicesCount || !DeviceContexts[ControllerId]._internal.connected)
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

	const auto NewForceFeedbackLeft{static_cast<unsigned char>(FMath::Max(Extra.ForceFeedbackLeftLarge, Extra.ForceFeedbackLeftSmall))};
	const auto NewForceFeedbackRight{static_cast<unsigned char>(FMath::Max(Extra.ForceFeedbackRightLarge, Extra.ForceFeedbackRightSmall))};

	auto& Output{OutputStates[ControllerId]};

	Extra.bOutputChanged |= Output.leftRumble != NewForceFeedbackLeft || Output.rightRumble != NewForceFeedbackRight;

	Output.leftRumble = NewForceFeedbackLeft;
	Output.rightRumble = NewForceFeedbackRight;
}

void FDsInputDevice::SetChannelValues(const int32 ControllerId, const FForceFeedbackValues& Values)
{
	if (ControllerId < 0 || ControllerId >= DsConstants::MaxDevicesCount || !DeviceContexts[ControllerId]._internal.connected)
	{
		return;
	}

	auto& Extra{ExtraStates[ControllerId]};

	Extra.ForceFeedbackLeftLarge = static_cast<uint8>(Values.LeftLarge * TNumericLimits<uint8>::Max());
	Extra.ForceFeedbackLeftSmall = static_cast<uint8>(Values.LeftSmall * TNumericLimits<uint8>::Max());
	Extra.ForceFeedbackRightLarge = static_cast<uint8>(Values.RightLarge * TNumericLimits<uint8>::Max());
	Extra.ForceFeedbackRightSmall = static_cast<uint8>(Values.RightSmall * TNumericLimits<uint8>::Max());

	const auto NewForceFeedbackLeft{static_cast<unsigned char>(FMath::Max(Extra.ForceFeedbackLeftLarge, Extra.ForceFeedbackLeftSmall))};
	const auto NewForceFeedbackRight{static_cast<unsigned char>(FMath::Max(Extra.ForceFeedbackRightLarge, Extra.ForceFeedbackRightSmall))};

	auto& Output{OutputStates[ControllerId]};

	Extra.bOutputChanged |= Output.leftRumble != NewForceFeedbackLeft || Output.rightRumble != NewForceFeedbackRight;

	Output.leftRumble = NewForceFeedbackLeft;
	Output.rightRumble = NewForceFeedbackRight;
}

void FDsInputDevice::SetDeviceProperty(const int32 ControllerId, const FInputDeviceProperty* Property)
{
	if (ControllerId < 0 || ControllerId >= DsConstants::MaxDevicesCount || !DeviceContexts[ControllerId]._internal.connected)
	{
		return;
	}

	auto& Output{OutputStates[ControllerId]};
	auto& Extra{ExtraStates[ControllerId]};

	if (Property->Name == FInputDeviceLightColorProperty::PropertyName())
	{
		const auto& LightColorProperty{static_cast<const FInputDeviceLightColorProperty&>(*Property)};

		Extra.bOutputChanged |= ProcessLightColorProperty(Output, LightColorProperty);
		return;
	}

	if (Property->Name == FInputDeviceTriggerResetProperty::PropertyName())
	{
		const auto& TriggerResetProperty{static_cast<const FInputDeviceTriggerResetProperty&>(*Property)};

		Extra.bOutputChanged |= ProcessTriggerResetProperty(Output.leftTriggerEffect, TriggerResetProperty,
		                                                    EInputDeviceTriggerMask::Left);

		Extra.bOutputChanged |= ProcessTriggerResetProperty(Output.rightTriggerEffect, TriggerResetProperty,
		                                                    EInputDeviceTriggerMask::Right);
		return;
	}

	if (Property->Name == FInputDeviceTriggerFeedbackProperty::PropertyName())
	{
		const auto& TriggerFeedbackProperty{static_cast<const FInputDeviceTriggerFeedbackProperty&>(*Property)};

		Extra.bOutputChanged |= ProcessTriggerFeedbackProperty(Output.leftTriggerEffect, TriggerFeedbackProperty,
		                                                       EInputDeviceTriggerMask::Left);

		Extra.bOutputChanged |= ProcessTriggerFeedbackProperty(Output.rightTriggerEffect, TriggerFeedbackProperty,
		                                                       EInputDeviceTriggerMask::Right);
		return;
	}

	if (Property->Name == FInputDeviceTriggerResistanceProperty::PropertyName())
	{
		const auto& TriggerResistanceProperty{static_cast<const FInputDeviceTriggerResistanceProperty&>(*Property)};

		Extra.bOutputChanged |= ProcessTriggerResistanceProperty(Output.leftTriggerEffect, TriggerResistanceProperty,
		                                                         EInputDeviceTriggerMask::Left);

		Extra.bOutputChanged |= ProcessTriggerResistanceProperty(Output.rightTriggerEffect, TriggerResistanceProperty,
		                                                         EInputDeviceTriggerMask::Right);
		return;
	}

	if (Property->Name == FInputDeviceTriggerVibrationProperty::PropertyName())
	{
		const auto& TriggerVibrationProperty{static_cast<const FInputDeviceTriggerVibrationProperty&>(*Property)};

		Extra.bOutputChanged |= ProcessTriggerVibrationProperty(Output.leftTriggerEffect, TriggerVibrationProperty,
		                                                        EInputDeviceTriggerMask::Left);

		Extra.bOutputChanged |= ProcessTriggerVibrationProperty(Output.rightTriggerEffect, TriggerVibrationProperty,
		                                                        EInputDeviceTriggerMask::Right);
	}
}

bool FDsInputDevice::IsGamepadAttached() const
{
	auto bResult{false};

	for (auto i{0}; i < DsConstants::MaxDevicesCount; i++)
	{
		bResult |= DeviceContexts[i]._internal.connected;
	}

	return bResult;
}

void FDsInputDevice::RefreshDevices()
{
	static unsigned int KnownDeviceIds[DsConstants::MaxDevicesCount];
	unsigned int KnowDevicesCount{0};

	for (auto i{0}; i < DsConstants::MaxDevicesCount; i++)
	{
		if (DeviceContexts[i]._internal.connected)
		{
			KnownDeviceIds[i] = DeviceContexts[i]._internal.uniqueID;
			KnowDevicesCount += 1;
		}
	}

	static DS5W::DeviceEnumInfo DeviceInfos[DsConstants::MaxDevicesCount];
	unsigned int DevicesCount{0};

	const auto EnumDevicesResult{
		enumUnknownDevices(DeviceInfos, DsConstants::MaxDevicesCount, KnownDeviceIds, KnowDevicesCount, &DevicesCount)
	};

	switch (EnumDevicesResult)
	{
		case DS5W_OK:
			break;

		case DS5W_E_INSUFFICIENT_BUFFER:
			DevicesCount = DsConstants::MaxDevicesCount;
			break;

		default:
			UE_LOG(LogFabulousDualSense, Warning, TEXT("Failed to enumerate devices: %s."),
			       DsUtility::ReturnValueToString(EnumDevicesResult).GetData());
			return;
	}

	auto& InputDeviceMapper{IPlatformInputDeviceMapper::Get()};
	TStaticBitArray<DsConstants::MaxDevicesCount> ProcessedDeviceIndexes;

	// First iteration: process devices reconnection and already connected devices.

	for (unsigned int DeviceIndex{0}; DeviceIndex < DevicesCount; DeviceIndex++)
	{
		for (auto ControllerId{0}; ControllerId < DsConstants::MaxDevicesCount; ControllerId++)
		{
			if (DeviceContexts[ControllerId]._internal.uniqueID == DeviceInfos[DeviceIndex]._internal.uniqueID)
			{
				ProcessedDeviceIndexes[DeviceIndex] = true;

				if (!DeviceContexts[ControllerId]._internal.connected)
				{
					ConnectDevice(InputDeviceMapper, DeviceInfos[DeviceIndex], ControllerId);
				}

				break;
			}
		}
	}

	// Second iteration: process the connection of new devices (without reusing the
	// IDs of disconnected devices to give them the opportunity to reconnect later).

	for (unsigned int DeviceIndex{0}; DeviceIndex < DevicesCount; DeviceIndex++)
	{
		if (ProcessedDeviceIndexes[DeviceIndex])
		{
			continue;
		}

		for (auto ControllerId{0}; ControllerId < DsConstants::MaxDevicesCount; ControllerId++)
		{
			if (DeviceContexts[ControllerId]._internal.uniqueID == 0)
			{
				ProcessedDeviceIndexes[DeviceIndex] = true;

				ConnectDevice(InputDeviceMapper, DeviceInfos[DeviceIndex], ControllerId);
				break;
			}
		}
	}

	// Third iteration: process the connection of new devices (reusing the IDs of
	// disconnected devices, because there are not enough unused IDs for new devices).

	for (unsigned int DeviceIndex{0}; DeviceIndex < DevicesCount; DeviceIndex++)
	{
		if (ProcessedDeviceIndexes[DeviceIndex])
		{
			continue;
		}

		for (auto ControllerId{0}; ControllerId < DsConstants::MaxDevicesCount; ControllerId++)
		{
			if (!DeviceContexts[ControllerId]._internal.connected)
			{
				ProcessedDeviceIndexes[DeviceIndex] = true;

				ConnectDevice(InputDeviceMapper, DeviceInfos[DeviceIndex], ControllerId);
				break;
			}
		}
	}
}

void FDsInputDevice::ConnectDevice(IPlatformInputDeviceMapper& InputDeviceMapper,
                                   DS5W::DeviceEnumInfo& DeviceInfo, const int32 ControllerId)
{
	UE_LOG(LogFabulousDualSense, Log, TEXT("New device found: %s, Connection: %s."),
	       DeviceInfo._internal.path, DsUtility::DeviceConnectionToString(DeviceInfo._internal.connection).GetData());

	const auto InitializeDeviceContextResult{initDeviceContext(&DeviceInfo, &DeviceContexts[ControllerId])};
	if (DS5W_SUCCESS(InitializeDeviceContextResult))
	{
		UE_LOG(LogFabulousDualSense, Log, TEXT("Device connected: %s."), DeviceInfo._internal.path);

		FMemory::Memzero(InputStates[ControllerId]);
		FMemory::Memzero(OutputStates[ControllerId]);
		FMemory::Memzero(ExtraStates[ControllerId]);

		auto PlatformUserId{PLATFORMUSERID_NONE};
		auto InputDeviceId{INPUTDEVICEID_NONE};
		InputDeviceMapper.RemapControllerIdToPlatformUserAndDevice(ControllerId, PlatformUserId, InputDeviceId);

		InputDeviceMapper.Internal_MapInputDeviceToUser(InputDeviceId, PlatformUserId, EInputDeviceConnectionState::Connected);
	}
	else
	{
		UE_LOG(LogFabulousDualSense, Warning, TEXT("Failed to initialize device context: %s, Device: %s."),
		       DsUtility::ReturnValueToString(InitializeDeviceContextResult).GetData(), DeviceInfo._internal.path);

		FMemory::Memzero(DeviceContexts[ControllerId]);
	}
}

void FDsInputDevice::DisconnectDevice(IPlatformInputDeviceMapper& InputDeviceMapper, const int32 ControllerId,
                                      const FPlatformUserId PlatformUserId, const FInputDeviceId InputDeviceId)
{
	auto& Context{DeviceContexts[ControllerId]};

	UE_LOG(LogFabulousDualSense, Log, TEXT("Device disconnected: %s."), Context._internal.devicePath);

	freeDeviceContext(&Context);

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

	const auto TouchAxisX{static_cast<int32>(NewTouch.x - PreviousTouch.x)};
	if (TouchAxisX != 0)
	{
		MessageHandler->OnControllerAnalog(AxisXKeyName, PlatformUserId, InputDeviceId, TouchAxisX);
	}

	const auto TouchAxisY{static_cast<int32>(NewTouch.y - PreviousTouch.y)};
	if (TouchAxisY != 0)
	{
		MessageHandler->OnControllerAnalog(AxisYKeyName, PlatformUserId, InputDeviceId, TouchAxisY);
	}

	if (GetDefault<UDsSettings>()->bEmitMouseEventsFromTouchpad)
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

bool FDsInputDevice::ProcessLightColorProperty(DS5W::DS5OutputState& Output, const FInputDeviceLightColorProperty& ColorProperty)
{
	const auto PreviousColor{Output.lightbar};

	if (ColorProperty.bEnable)
	{
		Output.lightbar.r = static_cast<unsigned char>(ColorProperty.Color.R);
		Output.lightbar.g = static_cast<unsigned char>(ColorProperty.Color.G);
		Output.lightbar.b = static_cast<unsigned char>(ColorProperty.Color.B);
	}
	else
	{
		Output.lightbar = {};
	}

	return Output.lightbar.r != PreviousColor.r || Output.lightbar.g != PreviousColor.g || Output.lightbar.b != PreviousColor.b;
}

bool FDsInputDevice::ProcessTriggerResetProperty(DS5W::TriggerEffect& TriggerEffect,
                                                 const FInputDeviceTriggerResetProperty& TriggerProperty,
                                                 const EInputDeviceTriggerMask TriggerMask)
{
	if (!EnumHasAnyFlags(TriggerProperty.AffectedTriggers, TriggerMask))
	{
		return false;
	}

	TriggerEffect.effectType = DS5W::TriggerEffectType::ReleaseAll;
	return true;
}

bool FDsInputDevice::ProcessTriggerFeedbackProperty(DS5W::TriggerEffect& TriggerEffect,
                                                    const FInputDeviceTriggerFeedbackProperty& TriggerProperty,
                                                    const EInputDeviceTriggerMask TriggerMask)
{
	if (!EnumHasAnyFlags(TriggerProperty.AffectedTriggers, TriggerMask))
	{
		return false;
	}

	const auto PreviousEffectType{TriggerEffect.effectType};
	const auto PreviousPosition{TriggerEffect.Continuous.startPosition};
	const auto PreviousForce{TriggerEffect.Continuous.force};

	TriggerEffect.effectType = DS5W::TriggerEffectType::ContinuousResitance;

	const auto* InputSettings{UInputPlatformSettings::Get()};

	const auto MaxPosition{static_cast<float>(InputSettings->MaxTriggerFeedbackPosition)};

	TriggerEffect.Continuous.startPosition =
		MaxPosition > 0.0f
			? static_cast<unsigned char>(TriggerProperty.Position / MaxPosition * TNumericLimits<unsigned char>::Max())
			: 0;

	const auto MaxStrength{static_cast<float>(InputSettings->MaxTriggerFeedbackStrength)};

	TriggerEffect.Continuous.force =
		MaxStrength > 0.0f
			? static_cast<unsigned char>(TriggerProperty.Strengh / MaxStrength * TNumericLimits<unsigned char>::Max())
			: 0;

	return TriggerEffect.effectType != PreviousEffectType ||
	       TriggerEffect.Continuous.startPosition != PreviousPosition ||
	       TriggerEffect.Continuous.force != PreviousForce;
}

bool FDsInputDevice::ProcessTriggerResistanceProperty(DS5W::TriggerEffect& TriggerEffect,
                                                      const FInputDeviceTriggerResistanceProperty& TriggerProperty,
                                                      const EInputDeviceTriggerMask TriggerMask)
{
	if (!EnumHasAnyFlags(TriggerProperty.AffectedTriggers, TriggerMask))
	{
		return false;
	}

	// Partially supported. FInputDeviceTriggerResistanceProperty::StartStrength
	// and FInputDeviceTriggerResistanceProperty::EndStrength are ignored.

	const auto PreviousEffectType{TriggerEffect.effectType};
	const auto PreviousStartPosition{TriggerEffect.Section.startPosition};
	const auto PreviousEndPosition{TriggerEffect.Section.endPosition};

	TriggerEffect.effectType = DS5W::TriggerEffectType::SectionResitance;

	static constexpr auto MaxPositionInverse{1.0f / 9.0f};

	TriggerEffect.Section.startPosition = static_cast<unsigned char>(
		TriggerProperty.StartPosition * MaxPositionInverse * TNumericLimits<unsigned char>::Max());

	TriggerEffect.Section.endPosition = static_cast<unsigned char>(
		TriggerProperty.EndPosition * MaxPositionInverse * TNumericLimits<unsigned char>::Max());

	return TriggerEffect.effectType != PreviousEffectType ||
	       TriggerEffect.Section.startPosition != PreviousStartPosition ||
	       TriggerEffect.Section.endPosition != PreviousEndPosition;
}

bool FDsInputDevice::ProcessTriggerVibrationProperty(DS5W::TriggerEffect& TriggerEffect,
                                                     const FInputDeviceTriggerVibrationProperty& TriggerProperty,
                                                     const EInputDeviceTriggerMask TriggerMask)
{
	if (!EnumHasAnyFlags(TriggerProperty.AffectedTriggers, TriggerMask))
	{
		return false;
	}

	// Not supported. At the moment, it is not possible to make this work as expected.

	// const auto PreviousEffectType{TriggerEffect.effectType};
	// const auto PreviousPosition{TriggerEffect.EffectEx.startPosition};
	// const auto PreviousFrequency{TriggerEffect.EffectEx.frequency};
	//
	// TriggerEffect.effectType = DS5W::TriggerEffectType::EffectEx;
	//
	// const auto* InputSettings{UInputPlatformSettings::Get()};
	//
	// const auto MaxPosition{static_cast<float>(InputSettings->MaxTriggerVibrationTriggerPosition)};
	//
	// TriggerEffect.EffectEx.startPosition =
	// 	MaxPosition > 0.0f
	// 		? static_cast<unsigned char>(TriggerProperty.TriggerPosition / MaxPosition * TNumericLimits<unsigned char>::Max())
	// 		: 0;
	//
	// TriggerEffect.EffectEx.keepEffect = true;
	// TriggerEffect.EffectEx.beginForce = 0;
	// TriggerEffect.EffectEx.middleForce = 0;
	// TriggerEffect.EffectEx.endForce = 0;
	//
	// const auto MaxFrequency{static_cast<float>(InputSettings->MaxTriggerVibrationFrequency)};
	//
	// TriggerEffect.EffectEx.frequency =
	// 	MaxFrequency > 0.0f
	// 		? static_cast<unsigned char>(TriggerProperty.VibrationFrequency / MaxFrequency * TNumericLimits<unsigned char>::Max())
	// 		: 0;
	//
	// return TriggerEffect.effectType != PreviousEffectType ||
	//        TriggerEffect.EffectEx.startPosition != PreviousPosition ||
	//        TriggerEffect.EffectEx.frequency != PreviousFrequency;

	return false;
}
