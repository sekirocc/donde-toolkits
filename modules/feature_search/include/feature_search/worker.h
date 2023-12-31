#pragma once

#include "donde/definitions.h"
#include "donde/feature_search/definitions.h"
#include "donde/feature_search/shard.h"

#include <iostream>
#include <unordered_map>



namespace donde_toolkits ::feature_search ::search_manager {

class Worker {
  public:
    // NOTE: we leave a blank implementation here, for MockWorker
    virtual ~Worker(){};

    virtual std::string GetWorkerID() = 0;

    virtual std::string GetAddress() = 0;

    virtual bool Ready() = 0;

    virtual uint64 GetFreeSpace() = 0;

    // ListShards report all shards this worker is serving.
    virtual std::vector<DBShard> ListShards() = 0;

    // ServeShard let the worker serve this shard, for its features' CRUD
    virtual RetCode ServeShard(Shard& shard) = 0;

    // CloseShard close db_id/shard_id.
    virtual RetCode CloseShard(const std::string& db_id, const std::string& shard_id) = 0;

    // AddFeatures to db_id/shard_id, delegate to remote worker.
    virtual std::vector<std::string> AddFeatures(const std::string& db_id,
                                                 const std::string& shard_id,
                                                 const std::vector<Feature>& fts)
        = 0;

    // Search feature in the worker. worker can have multiple dbs, multiple shards.
    // only search in the requested db.
    virtual std::vector<FeatureSearchItem> SearchFeature(const std::string& db_id,
                                                         const Feature& query, int topk)
        = 0;
};

using WorkerPtr = std::shared_ptr<Worker>;

} // namespace donde_toolkits::feature_search::search_manager
