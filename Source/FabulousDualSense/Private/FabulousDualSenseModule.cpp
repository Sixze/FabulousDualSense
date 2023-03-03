#include "FabulousDualSenseModule.h"

#include "DsInputDevice.h"

IMPLEMENT_MODULE(FFabulousDualSenseModule, FabulousDualSense)

#define LOCTEXT_NAMESPACE "FabulousDualSenseModule"

void FFabulousDualSenseModule::StartupModule()
{
	IInputDeviceModule::StartupModule();

	static const FName CategoryName{TEXTVIEW("DualSense")};

	EKeys::AddMenuCategoryDisplayInfo(CategoryName, LOCTEXT("Category", "DualSense"), FName{TEXTVIEW("GraphEditor.PadEvent_16x")});

	EKeys::AddKey({DsConstants::TouchpadKey, LOCTEXT("TouchpadKey", "DualSense Touchpad"), FKeyDetails::GamepadKey, CategoryName});
	EKeys::AddKey({DsConstants::LogoKey, LOCTEXT("LogoKey", "DualSense Logo"), FKeyDetails::GamepadKey, CategoryName});
	EKeys::AddKey({DsConstants::MuteKey, LOCTEXT("MuteKey", "DualSense Mute"), FKeyDetails::GamepadKey, CategoryName});

	// Touch 1.

	EKeys::AddKey({DsConstants::Touch1Key, LOCTEXT("Touch1Key", "DualSense Touch 1"), FKeyDetails::GamepadKey, CategoryName});

	EKeys::AddKey({
		DsConstants::Touch1AxisXKey, LOCTEXT("Touch1AxisXKey", "DualSense Touch 1 X-Axis"),
		FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::UpdateAxisWithoutSamples, CategoryName
	});

	EKeys::AddKey({
		DsConstants::Touch1AxisYKey, LOCTEXT("Touch1AxisYKey", "DualSense Touch 1 Y-Axis"),
		FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::UpdateAxisWithoutSamples, CategoryName
	});

	EKeys::AddPairedKey({
		                    DsConstants::Touch1AxisXYKey, LOCTEXT("Touch1AxisXYKey", "DualSense Touch 1 XY-Axis"),
		                    FKeyDetails::GamepadKey | FKeyDetails::Axis2D | FKeyDetails::UpdateAxisWithoutSamples, CategoryName
	                    }, DsConstants::Touch1AxisXKey, DsConstants::Touch1AxisYKey);

	// Touch 2.

	EKeys::AddKey({DsConstants::Touch2Key, LOCTEXT("Touch2Key", "DualSense Touch 2"), FKeyDetails::GamepadKey, CategoryName});

	EKeys::AddKey({
		DsConstants::Touch2AxisXKey, LOCTEXT("Touch2AxisXKey", "DualSense Touch 2 X-Axis"),
		FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::UpdateAxisWithoutSamples, CategoryName
	});

	EKeys::AddKey({
		DsConstants::Touch2AxisYKey, LOCTEXT("Touch2AxisYKey", "DualSense Touch 2 Y-Axis"),
		FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::UpdateAxisWithoutSamples, CategoryName
	});

	EKeys::AddPairedKey({
		                    DsConstants::Touch2AxisXYKey, LOCTEXT("Touch2AxisXYKey", "DualSense Touch 2 XY-Axis"),
		                    FKeyDetails::GamepadKey | FKeyDetails::Axis2D | FKeyDetails::UpdateAxisWithoutSamples, CategoryName
	                    }, DsConstants::Touch2AxisXKey, DsConstants::Touch2AxisYKey);
}

TSharedPtr<IInputDevice> FFabulousDualSenseModule::CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& MessageHandler)
{
	return MakeShared<FDsInputDevice>(MessageHandler);
}

#undef LOCTEXT_NAMESPACE
