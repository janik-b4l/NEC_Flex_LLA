#include "NEC_FlexAnalyzerSettings.h"
#include <AnalyzerHelpers.h>


NEC_FlexAnalyzerSettings::NEC_FlexAnalyzerSettings()
    : mInputChannel( UNDEFINED_CHANNEL ),
      mSOF_A( 9000 ),
      mSOF_B( 4500 ),
      mBit_Init(560),
      mBit_One(1690),
      mBit_Zero(560),
      mEOF(560),
      m64bit(1),
	mInputChannelInterface()
{

	mInputChannelInterface.SetTitleAndTooltip( "Channel", "Channel" );
    mSOF_A_Interface.SetTitleAndTooltip( "SOF pulse length 1", "Length of the first pulse" );
    mSOF_B_Interface.SetTitleAndTooltip( "SOF pulse length 2", "Length of the second pulse" );
    mBit_Init_Interface.SetTitleAndTooltip( "Bit inititation length", "Length of the beginning pulse for each bit" );
    mBit_One_Interface.SetTitleAndTooltip( "Bit logic 1 length", "pulse length of a logic 1" );
    mBit_Zero_Interface.SetTitleAndTooltip( "Bit logic 0 length", "pulse length of a logic 0" );
    mEOF_Interface.SetTitleAndTooltip( "EOF pulse length", "EOF pulse length" );
    m64bit_Interface.SetTitleAndTooltip( "16bit data ?", "extend the data and address length to 16bit" );

	mSOF_A_Interface.SetMin( 10 );
    mSOF_B_Interface.SetMin( 10 );
    mBit_Init_Interface.SetMin( 10 );
    mBit_One_Interface.SetMin( 10 );
    mBit_Zero_Interface.SetMin( 10 );
    mEOF_Interface.SetMin( 10 );


	mInputChannelInterface.SetChannel( mInputChannel );
    mSOF_A_Interface.SetInteger( mSOF_A );
    mSOF_B_Interface.SetInteger( mSOF_B );
    mBit_Init_Interface.SetInteger( mBit_Init );
    mBit_One_Interface.SetInteger( mBit_One );
    mBit_Zero_Interface.SetInteger( mBit_Zero );
    mEOF_Interface.SetInteger( mEOF );
    m64bit_Interface.SetValue( m64bit );


	AddInterface( &mInputChannelInterface );
    AddInterface( &mSOF_A_Interface );
    AddInterface( &mSOF_B_Interface );
    AddInterface( &mBit_Init_Interface );
    AddInterface( &mBit_One_Interface );
    AddInterface( &mBit_Zero_Interface );
    AddInterface( &mEOF_Interface );
    AddInterface( &m64bit_Interface );


	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	AddChannel( mInputChannel, "Serial", false );
}

NEC_FlexAnalyzerSettings::~NEC_FlexAnalyzerSettings()
{
}

bool NEC_FlexAnalyzerSettings::SetSettingsFromInterfaces()
{
	mInputChannel = mInputChannelInterface.GetChannel();

	ClearChannels();
	AddChannel( mInputChannel, "NEC Flex", true );
    mSOF_A = mSOF_A_Interface.GetInteger();
    mSOF_B = mSOF_B_Interface.GetInteger();
    mBit_Init = mBit_Init_Interface.GetInteger();
    mBit_One = mBit_One_Interface.GetInteger();
    mBit_Zero = mBit_Zero_Interface.GetInteger();
    mEOF = mEOF_Interface.GetInteger();
    m64bit = m64bit_Interface.GetValue();

	return true;
}

void NEC_FlexAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mInputChannelInterface.SetChannel( mInputChannel );
    mSOF_A_Interface.SetInteger( mSOF_A );
    mSOF_B_Interface.SetInteger( mSOF_B );
    mBit_Init_Interface.SetInteger( mBit_Init );
    mBit_One_Interface.SetInteger( mBit_One );
    mBit_Zero_Interface.SetInteger( mBit_Zero );
    mEOF_Interface.SetInteger( mEOF );
    m64bit_Interface.SetValue( m64bit );
    
}

void NEC_FlexAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	text_archive >> mInputChannel;
    text_archive >> mSOF_A;
    text_archive >> mSOF_B;
    text_archive >> mBit_Init;
    text_archive >> mBit_One;
    text_archive >> mBit_Zero;
    text_archive >> mEOF;
    text_archive >> m64bit;


	ClearChannels();
	AddChannel( mInputChannel, "NEC Flex", true );

	UpdateInterfacesFromSettings();
}

const char* NEC_FlexAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << mInputChannel;
    text_archive << mSOF_A;
    text_archive << mSOF_B;
    text_archive << mBit_Init;
    text_archive << mBit_One;
    text_archive << mBit_Zero;
    text_archive << mEOF;
    text_archive << m64bit;

	return SetReturnString( text_archive.GetString() );
}
