#ifndef NEC_FLEX_ANALYZER_SETTINGS
#define NEC_FLEX_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class NEC_FlexAnalyzerSettings : public AnalyzerSettings
{
public:
	NEC_FlexAnalyzerSettings();
	virtual ~NEC_FlexAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mInputChannel;
    U32 mSOF_A, mSOF_B;
    U32 mBit_Init, mBit_One, mBit_Zero;
    U32 mEOF;
    U32 m64bit;

protected:
	AnalyzerSettingInterfaceChannel	mInputChannelInterface;
	AnalyzerSettingInterfaceInteger	mSOF_A_Interface;
    AnalyzerSettingInterfaceInteger mSOF_B_Interface;
    AnalyzerSettingInterfaceInteger mBit_Init_Interface;
    AnalyzerSettingInterfaceInteger mBit_One_Interface;
    AnalyzerSettingInterfaceInteger mBit_Zero_Interface;
    AnalyzerSettingInterfaceInteger mEOF_Interface;
    AnalyzerSettingInterfaceBool m64bit_Interface;
};

#endif //NEC_FLEX_ANALYZER_SETTINGS
