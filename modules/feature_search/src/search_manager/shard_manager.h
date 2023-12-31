#pragma once

#include "donde/definitions.h"
#include "donde/feature_search/definitions.h"
#include "donde/feature_search/shard.h"
#include "donde/feature_search/worker.h"

// #include "spdlog/spdlog.h"

#include <iostream>
#include <unordered_map>

namespace donde_toolkits ::feature_search ::search_manager {

class ShardManager {

  public:
    virtual ~ShardManager() = default;

    virtual std::tuple<Shard*, bool> FindOrCreateWritableShard(std::string db_id, uint64 fts_count)
        = 0;

    virtual void LoadShards(std::string db_id) = 0;

    virtual std::vector<Shard*> ListShards(std::string db_id) = 0;

    virtual RetCode CloseShard(std::string db_id, std::string shard_id) = 0;

    virtual std::string CreateShard(DBShard shard_info) = 0;

    virtual RetCode UpdateShard(DBShard shard_info) = 0;
};
;

} // namespace donde_toolkits::feature_search::search_manager
