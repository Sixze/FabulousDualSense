#pragma once

#include <DualSenseWindows.h>

#include "DsConstants.h"
#include "IInputDevice.h"
#include "Containers/StaticArray.h"

enum class EInputDeviceTriggerMask : uint8;
struct FInputDeviceLightColorProperty;
struct FInputDeviceTriggerResetProperty;
struct FInputDeviceTriggerFeedbackProperty;
struct FInputDeviceTriggerResistanceProperty;
struct FInputDeviceTriggerVibrationProperty;

struct FABULOUSDUALSENSE_API FDsExtraState
{
	double ButtonsNextRepeatTime[DsConstants::ButtonsCount]{};

	uint8 ForceFeedbackLeftLarge{0};
	uint8 ForceFeedbackLeftSmall{0};

	uint8 ForceFeedbackRightLarge{0};
	uint8 ForceFeedbackRightSmall{0};

	uint8 bOutputChanged : 1 {true};
};

class FABULOUSDUALSENSE_API FDsInputDevice : public IInputDevice
{
private:
	TSharedPtr<FGenericApplicationMessageHandler> MessageHandler;

	float InitialButtonRepeatDelay{0.2f};

	float ButtonRepeatDelay{0.1f};

	TStaticArray<DS5W::DeviceContext, DsConstants::MaxDevicesCount> DeviceContexts{InPlace, DS5W::DeviceContext{}};

	TStaticArray<DS5W::DS5InputState, DsConstants::MaxDevicesCount> InputStates{InPlace, DS5W::DS5InputState{}};

	TStaticArray<DS5W::DS5OutputState, DsConstants::MaxDevicesCount> OutputStates{InPlace, DS5W::DS5OutputState{}};

	TStaticArray<FDsExtraState, DsConstants::MaxDevicesCount> ExtraStates;

public:
	explicit FDsInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& MessageHandler);

	virtual ~FDsInputDevice() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SendControllerEvents() override;

	virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& NewMessageHandler) override;

	virtual bool Exec(UWorld* World, const TCHAR* Command, FOutputDevice& Archive) override;

	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override;

	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values) override;

	virtual void SetDeviceProperty(int32 ControllerId, const FInputDeviceProperty* Property) override;

	virtual bool IsGamepadAttached() const override;

private:
	void RefreshDevices();

	void ConnectDevice(IPlatformInputDeviceMapper& InputDeviceMapper, DS5W::DeviceEnumInfo& DeviceInfo, int32 ControllerId);

	void DisconnectDevice(IPlatformInputDeviceMapper& InputDeviceMapper, int32 ControllerId,
	                      FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId);

	void ProcessStick(FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId,
	                  const FGamepadKeyNames::Type& KeyName, int8 PreviousValue, int8 NewValue) const;

	void ProcessButton(int32 ControllerId, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId,
	                   const FGamepadKeyNames::Type& KeyName, int32 ButtonIndex, bool bPreviousKeyDown, bool bNewKeyDown, double Time);

	void ProcessTouch(FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId, const FGamepadKeyNames::Type& AxisXKeyName,
	                  const FGamepadKeyNames::Type& AxisYKeyName, const DS5W::Touch& PreviousTouch, const DS5W::Touch& NewTouch) const;

	void ReleaseStick(FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId,
	                  const FGamepadKeyNames::Type& KeyName, int8 CurrentValue) const;

	void ReleaseButton(FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId,
	                   const FGamepadKeyNames::Type& KeyName, bool bPressed) const;

	static bool ProcessLightColorProperty(DS5W::DS5OutputState& Output, const FInputDeviceLightColorProperty& ColorProperty);

	static bool ProcessTriggerResetProperty(DS5W::TriggerEffect& TriggerEffect,
	                                        const FInputDeviceTriggerResetProperty& TriggerProperty,
	                                        EInputDeviceTriggerMask TriggerMask);

	static bool ProcessTriggerFeedbackProperty(DS5W::TriggerEffect& TriggerEffect,
	                                           const FInputDeviceTriggerFeedbackProperty& TriggerProperty,
	                                           EInputDeviceTriggerMask TriggerMask);

	static bool ProcessTriggerResistanceProperty(DS5W::TriggerEffect& TriggerEffect,
	                                             const FInputDeviceTriggerResistanceProperty& TriggerProperty,
	                                             EInputDeviceTriggerMask TriggerMask);

	static bool ProcessTriggerVibrationProperty(DS5W::TriggerEffect& TriggerEffect,
	                                            const FInputDeviceTriggerVibrationProperty& TriggerProperty,
	                                            EInputDeviceTriggerMask TriggerMask);
};
