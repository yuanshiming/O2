/// @file   CalibratorSpec.cxx

#include <vector>

#include "TGeoGlobalMagField.h"

#include "FairLogger.h"

#include "Framework/ControlService.h"
#include "Framework/ConfigParamRegistry.h"
#include "ITSWorkflow/TrackerSpec.h"
#include "DataFormatsITSMFT/CompCluster.h"
#include "DataFormatsITS/TrackITS.h"
#include "SimulationDataFormat/MCCompLabel.h"
#include "SimulationDataFormat/MCTruthContainer.h"
#include "DataFormatsITSMFT/ROFRecord.h"

#include "Field/MagneticField.h"
#include "DetectorsBase/GeometryManager.h"
#include "DetectorsBase/Propagator.h"
#include "ITSBase/GeometryTGeo.h"
#include "DetectorsCommonDataFormats/NameConf.h"

#include "ITSWorkflow/CalibratorSpec.h"

#include <vector>

#include <fmt/format.h>

#include <DPLUtils/RawParser.h>
#include <DPLUtils/DPLRawParser.h>

using namespace o2::framework;
using namespace o2::itsmft;
using namespace o2::header;

namespace o2
{
namespace its
{

using namespace o2::framework;

template <class Mapping>
ITSCalibrator<Mapping>::ITSCalibrator()
{
    mSelfName = o2::utils::concat_string(Mapping::getName(), "ITSCalibrator");
}

template <class Mapping>
void ITSCalibrator<Mapping>::init(InitContext& ic)
{
    LOGF(INFO, "ITSCalibrator init...", mSelfName);

    mDecoder = std::make_unique<RawPixelDecoder<ChipMappingITS>>();
    //mDecoder = new RawPixelDecoder<ChipMappingITS>();
    mChipsBuffer.resize(9);

    mDecoder->init();
    mDecoder->setFormat(GBTLink::NewFormat);    //Using RDHv6 (NewFormat)
    mDecoder->setVerbosity(0);

    //mChips.resize(12); //digital scan data now has 3 chips

    o2::base::GeometryManager::loadGeometry();
    mGeom = o2::its::GeometryTGeo::Instance();
    //mTimeFrameId = ctx.inputs().get<int>("G");
}

template <class Mapping>
void ITSCalibrator<Mapping>::run(ProcessingContext& pc)
{
    int chipId = 0;
    int TriggerId = 0;

    int lay, sta, ssta, mod, chip; //layer, stave, sub stave, module, chip
    
    mDecoder->startNewTF(pc.inputs());
    auto orig = Mapping::getOrigin();
    mDecoder->setDecodeNextAuto(false);

    DPLRawParser parser(pc.inputs());

    while (mDecoder->decodeNextTrigger()) {
        LOG(INFO) << "mTFCounter: " << mTFCounter << ", TriggerCounter in TF: " << TriggerId << ".";
        LOG(INFO) << "getNChipsFiredROF: " << mDecoder->getNChipsFiredROF() << ", getNPixelsFiredROF: " << mDecoder->getNPixelsFiredROF();
        TriggerId++;
        for(int loopi = 0; loopi<9; loopi++) {
            mChipDataBuffer = mDecoder->getNextChipData(mChipsBuffer);
            LOG(INFO) << "getChipID: " << mChipDataBuffer->getChipID() << ", getROFrame: " << mChipDataBuffer->getROFrame() << ", getTrigger: " << mChipDataBuffer->getTrigger();
            auto& pixels = mChipDataBuffer->getData();//type:
            for (auto& pixel : pixels) {
                if(pixel.isMasked()){
                    LOG(INFO) << "pixel Col = " << pixel.getCol() << ", pixel Row = " << pixel.getRow() << ", isMasked: " << pixel.isMasked();
                } else {
                    if ((pixel.getCol()==0) && (loopi==8)) {
                        LOG(INFO) << "No masked pixel in this trigger";
                    }
                }
            }
        //mChipDataBuffer = mDecoder->getNextChipData(mChipsBuffer);
        }
    }

    //decode the raw data and fill hit-map
    //while ((mChipDataBuffer = mDecoder->getNextChipData(mChipsBuffer))) {
    //  if (mChipDataBuffer) {
    //    const auto& pixels = mChipDataBuffer->getData();
    //    //LOG(INFO) << "pixels->getChipID(): " << pixels->getChipID();
    //    for (auto& pixel : pixels) {
    //      LOG(INFO) << "Col: " << pixel.getCol() << "Row: " << pixel.getRow();
    //      mGeom->getChipId(mChipDataBuffer->getChipID(), lay, sta, ssta, mod, chip);
    //      mHitNumberOfChip[lay][sta][ssta][mod][chip]++;
    //      if (mHitPixelID_Hash[lay][sta][ssta][mod][chip].find(pixel.getCol() * 1000 + pixel.getRow()) == mHitPixelID_Hash[lay][sta][ssta][mod][chip].end()) {
    //        mHitPixelID_Hash[lay][sta][ssta][mod][chip][pixel.getCol() * 1000 + pixel.getRow()] = 1;
    //      } else {
    //        mHitPixelID_Hash[lay][sta][ssta][mod][chip][pixel.getCol() * 1000 + pixel.getRow()]++;
    //      }
    //    }
    //  }
    //

    //LOG(INFO) << "mTFCounter: " << mTFCounter << ", getNLinks: " << mDecoder->getNLinks() << ", getVerbosity: " << mDecoder->getVerbosity();
    //LOG(INFO) << "getROFrame: " << mChipDataBuffer->getROFrame();
    //LOG(INFO) << "getNChipsFiredROF: " << mDecoder->getNChipsFiredROF() << ", getNPixelsFiredROF: " << mDecoder->getNPixelsFiredROF();

    //while (mChipDataBuffer = mDecoder->getNextChipData(mChipsBuffer)) {
    //    if(mChipDataBuffer){
    //        LOG(INFO) << "got chip data";
    //        const auto& pixels = mChipDataBuffer->getData();
    //    } else {}
    //}
    //while(mDecoder->decodeNextTrigger()) {
    //    LOG(INFO) << "mTFCounter: " << mTFCounter << ", TriggerCounter in TF: " << TriggerId << ".";
    //    LOG(INFO) << "getNChipsFiredROF: " << mDecoder->getNChipsFiredROF() << ", getNPixelsFiredROF: " << mDecoder->getNPixelsFiredROF();
    //    TriggerId++;
    //    mChipDataBuffer = mDecoder->getNextChipData(mChipsBuffer);
    //    const auto& pixels = mChipDataBuffer->getData();
    //    //while (mChipDataBuffer = mDecoder->getNextChipData(mChipsBuffer)) {
    //    //    LOG(INFO) << "getChipID: " << mChipDataBuffer->getChipID() << ".";
    //    //}
    //}

    //while((mChipDataBuffer = mDecoder->getNextChipData(mChipsBuffer))) {
    //    if(mChipDataBuffer) {
    //        LOG(INFO) << "ChipID: " << mChipDataBuffer->getChipID();
    //    } else {
    //        LOG(INFO) << "no mChipDataBuffer";
    //    }
    //}

    //LOG(INFO) << "getNChipsFiredROF: " << mDecoder->getNChipsFiredROF() << ", getNPixelsFiredROF: " << mDecoder->getNPixelsFiredROF();
    
    //mDecoder->decodeNextTrigger();

    //while((mChipDataBuffer = mDecoder->getNextChipData(mChipsBuffer))) {
    //    if(mChipDataBuffer) {
    //        LOG(INFO) << "ChipID: " << mChipDataBuffer->getChipID();
    //        //LOG(INFO) << "getNChipsFired: " << mDecoder->getNChipsFired() << ", getNPixelsFired: " << mDecoder->getNPixelsFired();
    //    } else {
    //        LOG(INFO) << "no mChipDataBuffer";
    //    }
    //}

    //mTimeFrameId = ctx.inputs().get<int>("G");

    //LOG(INFO) << "gbt link id: " << mTimeFrameId;    

    //while(mDecoder->decodeNextTrigger()) {
    //    LOG(INFO) << "mTFCounter: " << mTFCounter << ", TriggerCounter in TF: " << TriggerId << ".";
    //    LOG(INFO) << "getNChipsFiredROF: " << mDecoder->getNChipsFiredROF() << ", getNPixelsFiredROF: " << mDecoder->getNPixelsFiredROF();
    //    TriggerId++;    
    //}//this works with 'mDecoder->setDecodeNextAuto(false);'

    //while (mChipDataBuffer = mDecoder->getNextChipData(mChipsBuffer)) {
    //    LOG(INFO) << "getChipID: " << mChipDataBuffer->getChipID() << ".";
    //}

    //while ((mChipDataBuffer = mDecoder->getNextChipData(mChipsBuffer))) {
    //    if(mChipDataBuffer){
    //        LOG(INFO) << "got it";
    //    }
    //}
    //mDecoder->decodeNextTrigger();

    //mDecoder->printReport(true, true);

    TriggerId = 0;
    mTFCounter++;
}

template <class Mapping>
void ITSCalibrator<Mapping>::endOfStream(EndOfStreamContext& ec)
{
    LOGF(INFO, "endOfStream report:", mSelfName);
    if (mDecoder) {
        mDecoder->printReport(true, true);
    }
}

DataProcessorSpec getITSCalibratorSpec()
{
    std::vector<InputSpec> inputs;
    //inputs.emplace_back("ITS", "ITS", "RAWDATA", Lifetime::Timeframe);
    inputs.emplace_back("RAWDATA", ConcreteDataTypeMatcher{"ITS", "RAWDATA"}, Lifetime::Timeframe);

    // also, the "RAWDATA is taken from stf-builder wf, "
    // the Lifetime shoud probably not be 'Timeframe'... might be 'condition'
    // but first we manage to obtain the hit-map of one tf..., and then accumulate over the time axis
    // praying for some explaination on these "entities"...

    std::vector<OutputSpec> outputs;
    outputs.emplace_back("ITS", "CLUSTERSROF", 0, Lifetime::Timeframe);
    // how to create a hit-map, is there a lib in O2 for this purpose?


    auto orig = o2::header::gDataOriginITS;

    return DataProcessorSpec{
        "its-calibrator",
        inputs,
        outputs,
        AlgorithmSpec{adaptFromTask<ITSCalibrator<ChipMappingITS>>()},
        // here I assume ''calls'' init, run, endOfStream sequentially...
        Options{}
    };
}
} // namespace its
} // namespace o2