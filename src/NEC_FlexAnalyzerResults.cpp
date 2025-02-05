#include "NEC_FlexAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "NEC_FlexAnalyzer.h"
#include "NEC_FlexAnalyzerSettings.h"
#include <iostream>
#include <fstream>

NEC_FlexAnalyzerResults::NEC_FlexAnalyzerResults( NEC_FlexAnalyzer* analyzer, NEC_FlexAnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

NEC_FlexAnalyzerResults::~NEC_FlexAnalyzerResults()
{
}

void NEC_FlexAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
	ClearResultStrings();
Frame frame = GetFrame( frame_index );

char number_str[ 128 ];
char number_str2[ 128 ];
AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
AnalyzerHelpers::GetNumberString( frame.mData2, display_base, 8, number_str2, 128 );
if( !( frame.mFlags ) )
{
    std::string buf( "Address: " );
    buf.append( number_str );
    buf.append( "  Data: " );
    buf.append( number_str2 );        
    AddResultString(buf.data());
}
else if(frame.mFlags<= 3)
{
    std::string buf( "Address: " );
    buf.append( number_str );
    buf.append( "  Data: " );
    buf.append( number_str2 );
    if( frame.mFlags == 1 )
        buf.append( ", Addr mismatch" );
    else if( frame.mFlags == 2 )
        buf.append( ", Data mismatch" );
    else
        buf.append( ", Addr and Data mismatch" );
    AddResultString( buf.data() );
}
else
{
    AddResultString( "Error" );
    // todo: seperate errors
}
}

void NEC_FlexAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
	std::ofstream file_stream( file, std::ios::out );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	file_stream << "Time [s],Value" << std::endl;

	U64 num_frames = GetNumFrames();
	for( U32 i=0; i < num_frames; i++ )
	{
		Frame frame = GetFrame( i );
		
		char time_str[128];
		AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

		char number_str[128];
		AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );

		file_stream << time_str << "," << number_str << std::endl;

		if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
		{
			file_stream.close();
			return;
		}
	}

	file_stream.close();
}

void NEC_FlexAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
#ifdef SUPPORTS_PROTOCOL_SEARCH
	Frame frame = GetFrame( frame_index );
	ClearTabularText();

	char number_str[ 128 ];
    char number_str2[ 128 ];
    AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
    AnalyzerHelpers::GetNumberString( frame.mData2, display_base, 8, number_str2, 128 );
    if( !( frame.mFlags ) )
    {
        std::string buf( "Address: " );
        buf.append( number_str );
        buf.append( "  Data: " );
        buf.append( number_str2 );
        AddTabularText( buf.data() );
    }
    else
    {
        std::string buf( "Address: " );
        buf.append( number_str );
        buf.append( "  Data: " );
        buf.append( number_str2 );
        if( frame.mFlags == 1 )
            buf.append( ", Addr mismatch" );
        else if( frame.mFlags == 2 )
            buf.append( ", Data mismatch" );
        else
            buf.append( ", Addr and Data mismatch" );
        AddTabularText( buf.data() );
    }
#endif
}

void NEC_FlexAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
	//not supported

}

void NEC_FlexAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
	//not supported
}