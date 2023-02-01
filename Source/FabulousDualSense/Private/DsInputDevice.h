#pragma once

#include "DsUtility.h"
#include "IInputDevice.h"

class UDsSettings;

struct FABULOUSDUALSENSE_API FDsExtraState
{
	double ButtonsNextRepeatTime[DsConstants::ButtonsCount]{};

	uint8 ForceFeedbackLeftLarge{0};
	uint8 ForceFeedbackLeftSmall{0};

	uint8 ForceFeedbackRightLarge{0};
	uint8 ForceFeedbackRightSmall{0};

	bool bOutputChanged{true};
};

class FABULOUSDUALSENSE_API FDsInputDevice : public IInputDevice
{
private:
	TSharedPtr<FGenericApplicationMessageHandler> MessageHandler;

	TWeakObjectPtr<const UDsSettings> Settings;

	float InitialButtonRepeatDelay{0.2f};

	float ButtonRepeatDelay{0.1f};

	DS5W::DeviceContext DeviceContexts[DsConstants::MaxDevices]{};

	DS5W::DS5InputState InputStates[DsConstants::MaxDevices]{};

	DS5W::DS5OutputState OutputStates[DsConstants::MaxDevices]{};

	FDsExtraState ExtraStates[DsConstants::MaxDevices]{};

public:
	explicit FDsInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler);

	virtual ~FDsInputDevice() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SendControllerEvents() override;

	virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& NewMessageHandler) override;

	virtual bool Exec(UWorld* World, const TCHAR* Command, FOutputDevice& Archive) override;

	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override;

	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values) override;

	virtual bool IsGamepadAttached() const override;

private:
	void RefreshDevices();

	void DisconnectDevice(int32 ControllerId);

	void ProcessStick(int32 ControllerId, const FGamepadKeyNames::Type& KeyName, int8 PreviousValue, int8 NewValue) const;

	void ProcessButton(int32 ControllerId, const FGamepadKeyNames::Type& KeyName, int32 ButtonIndex,
	                   bool bPreviousKeyDown, bool bNewKeyDown, double Time);

	void ProcessTouch(int32 ControllerId, const FGamepadKeyNames::Type& AxisXKeyName, const FGamepadKeyNames::Type& AxisYKeyName,
	                  const DS5W::Touch& PreviousTouch, const DS5W::Touch& NewTouch) const;

	void ReleaseStick(int32 ControllerId, const FGamepadKeyNames::Type& KeyName, int8 CurrentValue) const;

	void ReleaseButton(int32 ControllerId, const FGamepadKeyNames::Type& KeyName, bool bPressed) const;
};
