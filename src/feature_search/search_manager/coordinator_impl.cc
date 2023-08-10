#include "coordinator_impl.h"

#include "donde/definitions.h"
#include "donde/feature_search/feature_topk_rank.h"
#include "donde/feature_search/search_manager/coordinator.h"
#include "donde/feature_search/simple_driver.h"
#include "fmt/format.h"
#include "shard.h"
#include "shard_impl.h"
#include "shard_manager_impl.h"
#include "worker_manager_impl.h"

#include <exception>
#include <memory>
#include <queue>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>

using namespace fmt;

namespace donde_toolkits ::feature_search ::search_manager {

CoordinatorImpl::CoordinatorImpl(const json& coor_config) : config(coor_config) {
    if (!coor_config.contains("workers")) {
        throw "json config missing workers list.";
    }
    if (!coor_config.contains("driver")) {
        throw "json config missing driver config.";
    }
    std::string driver_type = coor_config["driver"];
    if (driver_type != SEARCH_DRIVER_CASSANDRA || driver_type != SEARCH_DRIVER_SIMPLE) {
        throw fmt::format("driver {} is not supported.", driver_type);
    }

    _driver = std::make_shared<SimpleDriver>((std::string)coor_config["cassandra"]["addr"]);
    _shard_factory = std::make_shared<ShardFactoryImpl>();
    _shard_manager = std::make_shared<ShardManagerImpl>(*_driver, *_shard_factory);
    _worker_manager = std::make_shared<WorkerManagerImpl>();

    _worker_addrs = (std::vector<std::string>)coor_config["workers"];
};

CoordinatorImpl::~CoordinatorImpl(){};

void CoordinatorImpl::Start() {
    std::unordered_map<std::string, std::string> workers = load_workers_from_db();
    _worker_manager->LoadKnownWorkers(workers);

    load_shards_from_db();

    assign_worker_for_shards();
};

void CoordinatorImpl::Stop() { deinitialize_workers(); };

std::vector<DBItem> CoordinatorImpl::ListUserDBs() { return {}; };

// AddFeatures to this db, we need find proper shard to store these fts.
std::vector<std::string> CoordinatorImpl::AddFeatures(const std::string& db_id,
                                                      const std::vector<Feature>& fts) {
    auto [shard, new_created] = _shard_manager->FindOrCreateWritableShard(db_id, fts.size());

    // if newly created shard, it doesn't have a worker.
    if (new_created) {
        Worker* worker = find_worker_for_shard(shard);
        if (worker == nullptr) {
            spdlog::error("cannot find a worker for shard: {}", shard->GetShardID());
            return {};
        }
        _shard_manager->AssignWorkerToShard(shard, worker);
        // shard->AssignWorker(worker);
        // worker->ServeShard(shard->GetShardInfo());
    }

    return shard->AddFeatures(fts);
};

// RemoveFeatures from this db
RetCode CoordinatorImpl::RemoveFeatures(const std::string& db_id,
                                        const std::vector<std::string>& feature_ids) {
    // TODO
    return RetCode::RET_OK;
};

// SearchFeatures in this db. delegate to shards.
std::vector<FeatureSearchItem> CoordinatorImpl::SearchFeature(const std::string& db_id,
                                                              const Feature& query, int topk) {
    // enlarge search area
    int enlarge_topk = topk * 2;

    FeatureTopkRanking rank(query, topk);

    auto shards = _shard_manager->ListShards(db_id);

    // TODO group shards to search at once.
    // a db has many shards, we should not search one by one, as each search establish a worker
    // connection.
    for (auto& shard : shards) {
        auto searched = shard->SearchFeature(query, enlarge_topk);
        rank.FeedIn(searched);
    }

    std::vector<FeatureSearchItem> ret = rank.SortOut();

    return ret;
};

Worker* CoordinatorImpl::find_worker_for_shard(Shard* shard) {
    int64 free_space = INT_MIN;
    Worker* selected;

    for (auto& worker : _workers) {
        if (worker->GetFreeSpace() > free_space) {
            selected = worker.get();
            free_space = worker->GetFreeSpace();
        }
    }

    return selected;
};

void CoordinatorImpl::assign_worker_for_shards() {
    std::vector<DBItem> dbs = _shard_manager->ListUserDBs();

    for (auto& db : dbs) {
        auto shards = _shard_manager->ListShards(db.db_id);
        for (auto& shard : shards) {
            Worker* worker = find_worker_for_shard(shard);
            if (worker == nullptr) {
                spdlog::error("cannot find a worker for shard: {}", shard->GetShardID());
                continue;
            }
            _shard_manager->AssignWorkerToShard(shard, worker);
            // shard->AssignWorker(worker);
            // worker->ServeShard(shard->GetShardInfo());
        }
    }
};

void CoordinatorImpl::initialize_workers(){

};

void CoordinatorImpl::deinitialize_workers(){};

} // namespace donde_toolkits::feature_search::search_manager
