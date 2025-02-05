#ifndef NEC_FLEX_ANALYZER_RESULTS
#define NEC_FLEX_ANALYZER_RESULTS

#include <AnalyzerResults.h>

class NEC_FlexAnalyzer;
class NEC_FlexAnalyzerSettings;

class NEC_FlexAnalyzerResults : public AnalyzerResults
{
public:
	NEC_FlexAnalyzerResults( NEC_FlexAnalyzer* analyzer, NEC_FlexAnalyzerSettings* settings );
	virtual ~NEC_FlexAnalyzerResults();

	virtual void GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base );
	virtual void GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id );

	virtual void GenerateFrameTabularText(U64 frame_index, DisplayBase display_base );
	virtual void GeneratePacketTabularText( U64 packet_id, DisplayBase display_base );
	virtual void GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base );

protected: //functions

protected:  //vars
	NEC_FlexAnalyzerSettings* mSettings;
	NEC_FlexAnalyzer* mAnalyzer;
};

#endif //NEC_FLEX_ANALYZER_RESULTS
