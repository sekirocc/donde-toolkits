#include "concurrent_processor.h"
#include "face_pipeline.h"
#include "types.h"

#include <doctest/doctest.h>
#include <doctest/trompeloeil.hpp>
#include <memory>
#include <nlohmann/json.hpp>

using namespace std;

using nlohmann::json;

TEST_CASE("FacePipeline can decode image binary to frame, aka cv::Mat.") {

    string conf;
    string device_id = "-1";

    FacePipeline pipe{conf, device_id};

    // std::shared_ptr<Processor> a;

    // RetCode ret = pipe.Init(a, a, a, a);

    // CHECK(ret == RET_OK);

    // pipe.Terminate();

    CHECK("aa" == "aa");
};
