#ifndef NEC_FLEX_SIMULATION_DATA_GENERATOR
#define NEC_FLEX_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <string>
class NEC_FlexAnalyzerSettings;

class NEC_FlexSimulationDataGenerator
{
public:
	NEC_FlexSimulationDataGenerator();
	~NEC_FlexSimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, NEC_FlexAnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

protected:
	NEC_FlexAnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

protected:
	void CreateSerialByte();
	std::string mSerialText;
	U32 mStringIndex;

	SimulationChannelDescriptor mSerialSimulationData;

};
#endif //NEC_FLEX_SIMULATION_DATA_GENERATOR