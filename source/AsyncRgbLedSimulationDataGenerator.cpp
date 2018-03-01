#include "AsyncRgbLedSimulationDataGenerator.h"

#include <cmath> // for M_PI, cos
#include <iostream>

#include "AsyncRgbLedAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

// these values are from the WS2811, in high-speed mode
const int T0H_NSEC = 250;
const int T0L_NSEC = 1000;

const int T1H_NSEC = 600;
const int T1L_NSEC = 650;

const int RESET_NSEC = 25000;

AsyncRgbLedSimulationDataGenerator::AsyncRgbLedSimulationDataGenerator()
{
}

AsyncRgbLedSimulationDataGenerator::~AsyncRgbLedSimulationDataGenerator()
{
}

void AsyncRgbLedSimulationDataGenerator::Initialize( U32 simulation_sample_rate, AsyncRgbLedAnalyzerSettings* settings )
{
	// Initialize the random number generator with a literal seed to obtain repeatability
    // Change this for srand(time(NULL)) for "truly" random sequences
    // NOTICE rand() an srand() are *not* thread safe
    srand( 42 );

	mSimulationSampleRateHz = simulation_sample_rate;
	mSettings = settings;

	const U32 nSecPerSample = 1000000000 / mSimulationSampleRateHz;

	mSampleCounts[0][0] = T0H_NSEC / nSecPerSample;
	mSampleCounts[0][1] = T0L_NSEC / nSecPerSample;
	mSampleCounts[1][0] = T1H_NSEC / nSecPerSample;
	mSampleCounts[1][1] = T1L_NSEC / nSecPerSample;

	mLEDSimulationData.SetChannel( mSettings->mInputChannel );
	mLEDSimulationData.SetSampleRate( simulation_sample_rate );
	//the channel will be kept low by default in between operations.
	mLEDSimulationData.SetInitialBitState( BIT_LOW );
}

U32 AsyncRgbLedSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel )
{
	U64 adjusted_largest_sample_requested = 
	AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

	while( mLEDSimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested )
	{
		WriteReset();

		// six RGB-triple cascade between resets
		for (int t=0; t<6; ++t) {
			CreateRGBWord();
		}
	}

	*simulation_channel = &mLEDSimulationData;
	return 1;
}

void AsyncRgbLedSimulationDataGenerator::CreateRGBWord()
{
	const U32 rgb = RandomRGBValue();
	WriteRGBTriple( rgb >> 16 & 0xff, rgb >> 8 & 0xff, rgb & 0xff );
}

void AsyncRgbLedSimulationDataGenerator::WriteRGBTriple( U8 red, U8 green, U8 blue )
{
	WriteUIntData(red, 8);
	WriteUIntData(green, 8);
	WriteUIntData(blue, 8);
}

void AsyncRgbLedSimulationDataGenerator::WriteReset()
{
	//we assime that the channel is already low, and we will leave it low when we leave the rest function.
	const U32 nSecPerSample = 1000000000 / mSimulationSampleRateHz;

	mLEDSimulationData.Advance( RESET_NSEC / nSecPerSample );
}

void AsyncRgbLedSimulationDataGenerator::WriteUIntData( U32 data, U8 bit_count )
{
	U32 mask =  1 << (bit_count - 1);
	for( U32 bit=0; bit < bit_count; ++bit) {
		WriteBit(data & mask);
		mask = mask >> 1;
	}	
}

void AsyncRgbLedSimulationDataGenerator::WriteBit(bool b)
{
	//we assume that the channel is already low, and we will leave it low when we exit the write bit function.
	mLEDSimulationData.Transition();
	mLEDSimulationData.Advance( mSampleCounts[b][0] );
	mLEDSimulationData.Transition(); // go low
	mLEDSimulationData.Advance( mSampleCounts[b][1] );
}

U32 AsyncRgbLedSimulationDataGenerator::RandomRGBValue() const
{
	const U8 red = rand() % 0xff;
	const U8 green = rand() % 0xff;
	const U8 blue = rand() % 0xff;
	return (red << 16) | (green << 8) | blue;
}