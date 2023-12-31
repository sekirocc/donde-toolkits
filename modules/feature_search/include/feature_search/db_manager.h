#pragma once

#include "donde/definitions.h"
#include "nlohmann/json.hpp"

#include <map>
#include <memory>
#include <opencv2/core/hal/interface.h>



using json = nlohmann::json;

namespace donde_toolkits {

namespace feature_search {

class DBManager {

  public:
    DBManager(const json& config, const json& driver_config);
    ~DBManager() = default;

    RetCode DBNew();

    RetCode DBList();

    RetCode DBGet();

    RetCode DBDelete();
};

} // namespace feature_search

} // namespace donde_toolkits
