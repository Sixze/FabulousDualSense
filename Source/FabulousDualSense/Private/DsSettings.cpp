#include "DsSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DsSettings)

#define LOCTEXT_NAMESPACE "DsSettings"

UDsSettings::UDsSettings()
{
	CategoryName = TEXT("Plugins");
}

#if WITH_EDITOR
FText UDsSettings::GetSectionText() const
{
	return LOCTEXT("Section", "Fabulous DualSense");
}

FText UDsSettings::GetSectionDescription() const
{
	return LOCTEXT("SectionDescription", "Fabulous DualSense Settings");
}
#endif

#undef LOCTEXT_NAMESPACE
