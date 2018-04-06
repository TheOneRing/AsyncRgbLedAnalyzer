#include "MockResults.h"

#include <cassert>
#include <iostream> // REMOVE ME

#define D_PTR() \
    AnalyzerTest::MockResultData* d = reinterpret_cast<AnalyzerTest::MockResultData*>(mData);

AnalyzerResults::AnalyzerResults()
{
    auto mock = new AnalyzerTest::MockResultData;
    mData = reinterpret_cast<AnalyzerResultsData*>(mock);
}

AnalyzerResults::~AnalyzerResults()
{

}

U64 AnalyzerResults::AddFrame(const Frame &frame)
{
    D_PTR();
    return d->AddFrame(frame);
}

void AnalyzerResults::CommitResults()
{
    D_PTR();
    d->mCommitFrames.push_back(d->CurrentFrame());
}

U64 AnalyzerResults::CommitPacketAndStartNewPacket()
{
    D_PTR();
    d->mPacketStartFrames.push_back(d->NextFrame());
    return d->CurrentFrame();
}

Frame AnalyzerResults::GetFrame(U64 frame_id)
{
    D_PTR();
    if (frame_id >= d->CurrentFrame()) {
        assert(false);
    }

    return d->GetFrame(frame_id);
}

U64 AnalyzerResults::GetPacketContainingFrameSequential(U64 frame_id)
{
    return 0;
}

U64 AnalyzerResults::GetNumFrames()
{
    D_PTR();
    return d->CurrentFrame();
}

void AnalyzerResults::ClearResultStrings()
{
    D_PTR();
    d->mStrings.clear();
}

void AnalyzerResults::AddResultString( const char* str1, const char* str2, const char* str3, const char* str4, const char* str5, const char* str6)
{
    // concatenate strings
    std::string s(str1);
    if (str2) s += str2;
    if (str3) s += str3;

    D_PTR();
    d->AddString(s);
}

void AnalyzerResults::ClearTabularText()
{

}

void AnalyzerResults::AddTabularText(const char* str1, const char* str2, const char* str3 , const char* str4, const char* str5 , const char* str6)
{


}

void AnalyzerResults::AddChannelBubblesWillAppearOn( const Channel& channel )
{

}

bool AnalyzerResults::UpdateExportProgressAndCheckForCancel( U64 completed_frames, U64 total_frames )
{
    D_PTR();
    return d->mCancelled;
}

//////////////////////////////////////////////////////////////////////////////

Frame::Frame()
{
}

Frame::Frame(const Frame& f)
{
}

Frame::~Frame()
{
}

bool Frame::HasFlag( U8 flag )
{
    return mFlags & flag;
}

namespace {

// helper class to access mData outside the class. Compiler will flatten
// all this code into nothing
class DataExtractor : public AnalyzerResults {
public:
    AnalyzerTest::MockResultData* mockData() {
        return reinterpret_cast<AnalyzerTest::MockResultData*>(mData);
    }
};

} // of anonymous namespace

namespace AnalyzerTest
{

MockResultData* MockResultData::MockFromResults(AnalyzerResults *results)
{
    return static_cast<DataExtractor*>(results)->mockData();
}

U64 MockResultData::AddFrame(const Frame &f)
{
    mFrames.push_back(f);
    return CurrentFrame();
}

const Frame& MockResultData::GetFrame(U64 index) const
{
    return mFrames.at(index);
}

U64 MockResultData::CurrentFrame() const
{
    return mFrames.size() - 1;
}

U64 MockResultData::NextFrame() const
{
    return mFrames.size();
}

void MockResultData::AddString(const std::string &s)
{
    mStrings.push_back({CurrentFrame(), s});
}

void MockResultData::SetCancelled(bool cancelled)
{
    mCancelled = cancelled;
}

auto MockResultData::GetFrameRangeForPacket(U64 packetIndex) const -> FrameRange
{
    assert(packetIndex < mPacketStartFrames.size());
    U64 endFrame = (packetIndex < (mPacketStartFrames.size() - 1) ?
                        mPacketStartFrames.at(packetIndex + 1) - 1 : // frame preceeding start of next packet
                        mFrames.size() - 1); // final valid frame
    return std::make_pair(mPacketStartFrames.at(packetIndex), endFrame);
}


} // of namespace

