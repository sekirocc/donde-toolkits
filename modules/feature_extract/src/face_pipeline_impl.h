#pragma once

#include "donde/definitions.h"
#include "donde/feature_extract/processor.h"
// #include "faiss/Index2Layer.h"
#include "nlohmann/json.hpp"

#include <map>

using namespace Poco;


using json = nlohmann::json;

namespace donde_toolkits ::feature_extract {

class FacePipelineImpl {
  public:
    FacePipelineImpl(const json& config);

    const json& GetConfig() { return _config; };

    RetCode Init(Processor* detector, Processor* landmarks, Processor* aligner, Processor* feature);

    RetCode Terminate();

    std::shared_ptr<Frame> Decode(const std::vector<uint8_t>& image_data);

    std::shared_ptr<DetectResult> Detect(const std::shared_ptr<Frame> frame);

    std::shared_ptr<LandmarksResult> Landmarks(const std::shared_ptr<DetectResult> detect_result);

    std::shared_ptr<AlignerResult> Align(const std::shared_ptr<LandmarksResult> landmarks_result);

    std::shared_ptr<FeatureResult> Extract(std::shared_ptr<AlignerResult> aligner_result);

  private:
    json _config;

    std::shared_ptr<Processor> _detectorProcessor;
    std::shared_ptr<Processor> _landmarksProcessor;
    std::shared_ptr<Processor> _alignerProcessor;
    std::shared_ptr<Processor> _featureProcessor;
};

} // namespace donde_toolkits::feature_extract
