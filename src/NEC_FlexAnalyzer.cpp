#include "NEC_FlexAnalyzer.h"
#include "NEC_FlexAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

NEC_FlexAnalyzer::NEC_FlexAnalyzer()
:	Analyzer2(),  
	mSettings(),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( &mSettings );
}

NEC_FlexAnalyzer::~NEC_FlexAnalyzer()
{
	KillThread();
}

void NEC_FlexAnalyzer::SetupResults()
{
	// SetupResults is called each time the analyzer is run. Because the same instance can be used for multiple runs, we need to clear the results each time.
	mResults.reset(new NEC_FlexAnalyzerResults( this, &mSettings ));
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings.mInputChannel );
}

void NEC_FlexAnalyzer::WorkerThread()
{
    mSerial = GetAnalyzerChannelData( mSettings.mInputChannel );
    U64 times[ 133 ]; // 128 transitions for data +2 SOF + 1 EOF
    U8 index = 0;
    U8 index_data = 0;
    U64 diff = 0;
    U8 numbits = 0;
    U64 result = 0;

    U8 index_max = 0;
    if( mSettings.m64bit )
        index_max = 128 + 2; // 64*2 +2 (16 bit *4 = 64 bit)
    else
        index_max = 64 + 2; // 32*2 +2 (8bit *4 = 32 bit)

    // mSerial->AdvanceToNextEdge();
    times[ 0 ] = mSerial->GetSampleNumber();
    times[ 1 ] = mSerial->GetSampleOfNextEdge();
    double timebase_us = ( 1e6 / GetSampleRate() );
    diff = times[ 1 ] - times[ 0 ];
    while( !( ( diff * timebase_us > ( mSettings.mSOF_A - mSettings.mSOF_A / 10 ) ) &&
              ( diff * timebase_us < ( mSettings.mSOF_A + mSettings.mSOF_A / 10 ) ) ) )
    {
        if( mSerial->DoMoreTransitionsExistInCurrentData() )
        {
            mSerial->AdvanceToNextEdge();
            times[ 0 ] = mSerial->GetSampleNumber();
            times[ 1 ] = mSerial->GetSampleOfNextEdge();
            diff = times[ 1 ] - times[ 0 ];
        }
        else
            return; // no starting pulse found
    }
    mSerial->AdvanceToNextEdge();
    if( mSerial->DoMoreTransitionsExistInCurrentData() )
    {
        mSerial->AdvanceToNextEdge();
        times[ 2 ] = mSerial->GetSampleNumber();
    }
    else
    {
        // incomplete sof
        Frame eframe;
        eframe.mData1 = 0;
        eframe.mFlags = 4;
        eframe.mStartingSampleInclusive = times[ 0 ];
        eframe.mEndingSampleInclusive = times[ 1 ];
        mResults->AddFrame( eframe );
        mResults->AddMarker( times[ 1 ], AnalyzerResults::ErrorDot, mSettings.mInputChannel );
        mResults->CommitResults();
        ReportProgress( mSerial->GetSampleNumber() );
        return;

    }
    diff = times[ 2 ] - times[ 1 ];
    if( !( ( diff * timebase_us > ( mSettings.mSOF_B - mSettings.mSOF_B / 10 ) ) &&
           ( diff * timebase_us < ( mSettings.mSOF_B + mSettings.mSOF_B / 10 ) ) ) )
    {
        mResults->AddMarker( times[ 0 ], AnalyzerResults::ErrorDot, mSettings.mInputChannel );
        mResults->AddMarker( times[ 2 ], AnalyzerResults::ErrorDot, mSettings.mInputChannel );      
        // SOF is invalid (4.5ms wrong)
        Frame eframe;
        eframe.mData1 = 0;
        eframe.mFlags = 5;
        eframe.mStartingSampleInclusive = times[ 0 ];
        eframe.mEndingSampleInclusive = times[ 2 ];
        mResults->AddFrame( eframe );
        mResults->CommitResults();
        ReportProgress( mSerial->GetSampleNumber() );
        return;
    }
    mResults->AddMarker( times[ 0 ], AnalyzerResults::Start, mSettings.mInputChannel );
    mResults->AddMarker( times[ 2 ], AnalyzerResults::Start, mSettings.mInputChannel );
    index = 3;
    while( ( index <= index_max ) && ( mSerial->DoMoreTransitionsExistInCurrentData() ) )
    {
        times[ index ] = mSerial->GetSampleOfNextEdge();
        diff = times[ index ] - times[ index - 1 ];

        // initial bit pulse
        if( !( ( diff * timebase_us > ( mSettings.mBit_Init - mSettings.mBit_Init / 10 ) ) &&
               ( diff * timebase_us < ( mSettings.mBit_Init + mSettings.mBit_Init / 10 ) ) ) )
        {
            Frame eframe;
            eframe.mData1 = 0;
            eframe.mFlags = 6;
            eframe.mStartingSampleInclusive = times[ 0 ];
            eframe.mEndingSampleInclusive = times[ index-1 ];
            mResults->AddFrame( eframe );
            mResults->AddMarker( times[ index - 1 ], AnalyzerResults::ErrorX, mSettings.mInputChannel );
            mResults->AddMarker( times[ index ], AnalyzerResults::ErrorX, mSettings.mInputChannel );
            mResults->CommitResults();
            ReportProgress( mSerial->GetSampleNumber() );
            return;
        }
        mSerial->AdvanceToNextEdge();
        index++;
        // this is either a 560us or 1120us width
        if( !( mSerial->DoMoreTransitionsExistInCurrentData() ) )
        {
            // incomplete bit
            mResults->AddMarker( mSerial->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings.mInputChannel );
            Frame eframe;
            eframe.mData1 = 0;
            eframe.mFlags = 7;
            eframe.mStartingSampleInclusive = times[ 0 ];
            eframe.mEndingSampleInclusive = mSerial->GetSampleNumber();
            mResults->AddFrame( eframe );
            mResults->CommitResults();
            ReportProgress( mSerial->GetSampleNumber() );
            return;
        }
        times[ index ] = mSerial->GetSampleOfNextEdge();
        diff = times[ index ] - times[ index - 1 ];
        if( ( diff * timebase_us > ( mSettings.mBit_Zero - mSettings.mBit_Zero / 10 ) ) &&
            ( diff * timebase_us < ( mSettings.mBit_Zero + mSettings.mBit_Zero / 10 ) ) )
        {
            // this is a logic 0
            numbits++;
            mResults->AddMarker( times[ index - 1 ], AnalyzerResults ::Dot, mSettings.mInputChannel );
            mResults->AddMarker( times[index], AnalyzerResults::Zero, mSettings.mInputChannel );    
        }
        else if( ( diff * timebase_us > ( mSettings.mBit_One - mSettings.mBit_One / 10 ) ) &&
                 ( diff * timebase_us < ( mSettings.mBit_One + mSettings.mBit_One / 10 ) ) )
        {
            // this is a logic 1
            result = result | ( static_cast<unsigned long long>( 1 ) << numbits );
            numbits++;
            mResults->AddMarker( times[ index - 1 ], AnalyzerResults ::Dot, mSettings.mInputChannel );
            mResults->AddMarker( times[ index ], AnalyzerResults::One, mSettings.mInputChannel );         
        }
        else
        {
            // this is wrong
            mResults->AddMarker( times[ index - 1 ], AnalyzerResults::ErrorX, mSettings.mInputChannel );
            mResults->AddMarker( times[index], AnalyzerResults::ErrorX, mSettings.mInputChannel );
            Frame eframe;
            eframe.mData1 = 0;
            eframe.mFlags = 8;
            eframe.mStartingSampleInclusive = times[ index - 1 ];
            eframe.mEndingSampleInclusive = times[ index ];
            mResults->AddFrame( eframe );
            mResults->CommitResults();
            ReportProgress( mSerial->GetSampleNumber() );
            return;
        }

        mSerial->AdvanceToNextEdge();
        index++;
    }

    if( index == index_max + 1 )
    {
        // this is the last transition, it should be 560us
        if( !( mSerial->DoMoreTransitionsExistInCurrentData() ) )
        {
            // EOM is missing
            mResults->AddMarker( mSerial->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings.mInputChannel );
            Frame eframe;
            eframe.mData1 = 0;
            eframe.mFlags = 9;
            eframe.mStartingSampleInclusive = times[ 0 ];
            eframe.mEndingSampleInclusive = mSerial->GetSampleNumber();
            mResults->AddFrame( eframe );
            mResults->CommitResults();
            ReportProgress( mSerial->GetSampleNumber() );
            return;
        }
        else
        {
            diff = mSerial->GetSampleOfNextEdge() - times[ index-1 ];
            times[ index ] = mSerial->GetSampleOfNextEdge();
            if( ( diff * timebase_us > ( mSettings.mEOF - mSettings.mEOF / 10 ) ) &&
                ( diff * timebase_us < ( mSettings.mEOF + mSettings.mEOF / 10 ) ) )
            {
                if( mSettings.m64bit )
                {
                    // message is complete(64bit)
                    mResults->AddMarker( times[ index ], AnalyzerResults::Stop, mSettings.mInputChannel );
                    Frame frame;
                    U16 addr = 0;
                    U16 addr_inv = 0;
                    U16 data = 0;
                    U16 data_inv = 0;

                    addr = result & 0xFFFF;
                    addr_inv = ( result >> 16 ) & 0xFFFF;
                    data = ( result >> 32 ) & 0xFFFF;
                    data_inv = ( result >> 48 ) & 0xFFFF;
                    frame.mFlags = 0;
                    if( ( addr != static_cast<U16>( ~addr_inv ) ) )
                    {
                        frame.mFlags |= 1;
                    }
                    if( data != static_cast<U16>( ~data_inv ) )
                    {
                        frame.mFlags |= 2;
                    }

                    frame.mData1 = addr;
                    frame.mData2 = data;
                    frame.mStartingSampleInclusive = times[ 0 ];
                    frame.mEndingSampleInclusive = times[ index_max+1 ];
                    mSerial->AdvanceToNextEdge();
                    mResults->AddFrame( frame );

                    // New FrameV2 code.
                    FrameV2 frame_v2;
                    // you can add any number of key value pairs. Each will get it's own column in the data table.
                    frame_v2.AddInteger( "numrecv", numbits );
                    // frame_v2.AddInteger( "red", result.mRGB.red );
                    // frame_v2.AddInteger( "green", result.mRGB.green );
                    // frame_v2.AddInteger( "blue", result.mRGB.blue );
                    //  This actually saves your new FrameV2. In this example, we just copy the same start and end sample number from Frame
                    //  V1 above. The second parameter is the frame "type". Any string is allowed.
                    mResults->AddFrameV2( frame_v2, "nec_frame", times[ 0 ], times[ index_max ] );
                    mResults->CommitResults();
                    ReportProgress( mSerial->GetSampleNumber() );
                }
                else
                {
                    // message is complete(32bit)
                    mResults->AddMarker( times[ index ], AnalyzerResults::Stop, mSettings.mInputChannel );
                    Frame frame;
                    U8 addr = 0;
                    U8 addr_inv = 0;
                    U8 data = 0;
                    U8 data_inv = 0;

                    addr = result & 0xFF;
                    addr_inv = ( result >> 8 ) & 0xFF;
                    data = ( result >> 16 ) & 0xFF;
                    data_inv = ( result >> 24 ) & 0xFF;
                    frame.mFlags = 0;
                    if( ( addr != static_cast<U8>( ~addr_inv ) ) )
                    {
                        frame.mFlags |= 1;
                    }
                    if( data != static_cast<U8>( ~data_inv ) )
                    {
                        frame.mFlags |= 2;
                    }

                    frame.mData1 = addr;
                    frame.mData2 = data;
                    frame.mStartingSampleInclusive = times[ 0 ];
                    frame.mEndingSampleInclusive = times[ index_max+1 ];
                    mSerial->AdvanceToNextEdge();
                    mResults->AddFrame( frame );

                    // New FrameV2 code.
                    FrameV2 frame_v2;
                    // you can add any number of key value pairs. Each will get it's own column in the data table.
                    frame_v2.AddInteger( "numrecv", numbits );
                    // frame_v2.AddInteger( "red", result.mRGB.red );
                    // frame_v2.AddInteger( "green", result.mRGB.green );
                    // frame_v2.AddInteger( "blue", result.mRGB.blue );
                    //  This actually saves your new FrameV2. In this example, we just copy the same start and end sample number from Frame
                    //  V1 above. The second parameter is the frame "type". Any string is allowed.
                    mResults->AddFrameV2( frame_v2, "nec_frame", times[ 0 ], times[ index_max ] );
                    mResults->CommitResults();
                    ReportProgress( mSerial->GetSampleNumber() );
                }
            }
            else
            {
                // EOM is invalid
                mResults->AddMarker( mSerial->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings.mInputChannel );
                mResults->AddMarker( mSerial->GetSampleOfNextEdge(), AnalyzerResults::ErrorX, mSettings.mInputChannel );
                Frame eframe;
                eframe.mData1 = 0;
                eframe.mFlags = 10;
                eframe.mStartingSampleInclusive = times[ index_max ];
                eframe.mEndingSampleInclusive = mSerial->GetSampleNumber();
                mResults->AddFrame( eframe );
                mResults->AddMarker( times[ 1 ], AnalyzerResults::ErrorDot, mSettings.mInputChannel );
                mResults->CommitResults();
                ReportProgress( mSerial->GetSampleNumber() );
                return;
                ReportProgress( mSerial->GetSampleNumber() );
            }

                
        }
    }

}

bool NEC_FlexAnalyzer::NeedsRerun()
{
	return false;
}

U32 NEC_FlexAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), &mSettings );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 NEC_FlexAnalyzer::GetMinimumSampleRateHz()
{
    return (( 1e6 / mSettings.mBit_Init )*4);
}

const char* NEC_FlexAnalyzer::GetAnalyzerName() const
{
	return "NEC Flex";
}

const char* GetAnalyzerName()
{
	return "NEC Flex";
}

Analyzer* CreateAnalyzer()
{
	return new NEC_FlexAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}