#ifndef NEC_FLEX_ANALYZER_H
#define NEC_FLEX_ANALYZER_H

#include <Analyzer.h>
#include "NEC_FlexAnalyzerSettings.h"
#include "NEC_FlexAnalyzerResults.h"
#include "NEC_FlexSimulationDataGenerator.h"
#include <memory>

class ANALYZER_EXPORT NEC_FlexAnalyzer : public Analyzer2
{
public:
	NEC_FlexAnalyzer();
	virtual ~NEC_FlexAnalyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //vars
	NEC_FlexAnalyzerSettings mSettings;
	std::unique_ptr<NEC_FlexAnalyzerResults> mResults;
	AnalyzerChannelData* mSerial;

	NEC_FlexSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;

	//Serial analysis vars:
	U32 mSampleRateHz;
	U32 mStartOfStopBitOffset;
	U32 mEndOfStopBitOffset;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //NEC_FLEX_ANALYZER_H
