#pragma once

#include "IInputDeviceModule.h"

class FABULOUSDUALSENSE_API FFabulousDualSenseModule : public IInputDeviceModule
{
public:
	virtual void StartupModule() override;

	virtual TSharedPtr<IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& MessageHandler) override;
};
