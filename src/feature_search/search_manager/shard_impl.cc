#include "shard_impl.h"

#include "donde/definitions.h"
#include "shard_manager.h"

#include <exception>
#include <memory>
#include <mutex>
#include <spdlog/spdlog.h>

namespace donde_toolkits ::feature_search ::search_manager {

const int WAIT_MSG_INTERVAL_MS = 3000;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Shard
////////////////////////////////////////////////////////////////////////////////////////////////////////////

ShardImpl::ShardImpl(ShardManager* manager, DBShard shard_info)
    : _shard_info(shard_info),
      _shard_id(shard_info.shard_id),
      _db_id(shard_info.db_id),
      _shard_mgr(manager),
      _is_stopped(true) {

    Start();
};

ShardImpl::~ShardImpl() { Stop(); };

void ShardImpl::Start() {
    std::lock_guard<std::mutex> l(_thread_mu);

    if (!_is_stopped.load()) {
        spdlog::warn("shard already is running, double Start??.");
        return;
    }

    // create new thread
    _loop_thread.reset(new std::thread(&ShardImpl::loop, std::ref(*this)));
    _channel.reset(new MsgChannel());

    _is_stopped.store(false);
};

void ShardImpl::Stop() {
    std::lock_guard<std::mutex> l(_thread_mu);

    if (_is_stopped.load()) {
        spdlog::warn("shard already is stopped, double Stop??.");
        return;
    }

    _is_stopped.store(true);
    _channel->wakeUpAll();

    try {
        if (_loop_thread) {
            _loop_thread->join();
            // release thread.
            _loop_thread.reset();
        }
    } catch (const std::exception& exc) {
        spdlog::warn("catch exc when join thread {}.", exc.what());
    }
    // release channel
    _channel.reset();

    std::cout << "after join" << std::endl;
};

//////////////////////////////////////////////////////////////////////////////////
// Feature management
//////////////////////////////////////////////////////////////////////////////////

// Assign a worker for this shard.
RetCode ShardImpl::AssignWorker(Worker* worker) {
    if (HasWorker()) {
        spdlog::error("shard already has worker, double AssignWorker.");
        return RetCode::RET_ERR;
    }
    if (worker == nullptr) {
        spdlog::error("assigned null worker?");
        return RetCode::RET_ERR;
    }

    _worker = worker;
    _worker_id = worker->GetWorkerID();

    auto msg = NewWorkMessage(shardOp{assignWorkerReqType});
    _channel->enqueueNotification(msg);
    auto output = msg->waitResponse();
    auto value = std::static_pointer_cast<assignWorkerRsp>(output.valuePtr);

    return RetCode::RET_OK;
};

// AddFeatures to this shard, delegate to worker client to do the actual storage.
std::vector<std::string> ShardImpl::AddFeatures(const std::vector<Feature>& fts) {
    if (!HasWorker()) {
        spdlog::error("shard has no worker, AssignWorker first.");
        return {};
    }

    ;
    auto input = shardOp{
        .valueType = addFeaturesReqType,
        .valuePtr = std::shared_ptr<addFeaturesReq>(new addFeaturesReq{fts}),
    };

    auto msg = NewWorkMessage(input);
    _channel->enqueueNotification(msg);
    auto output = msg->waitResponse();
    auto value = std::static_pointer_cast<addFeaturesRsp>(output.valuePtr);
    return value->feature_ids;
};

// SearchFeature in this shard, delegate to worker client to do the actual search.
std::vector<FeatureSearchItem> ShardImpl::SearchFeature(const Feature& query, int topk) {
    if (!HasWorker()) {
        spdlog::error("shard has no worker, AssignWorker first.");
        return {};
    }

    auto msg = NewWorkMessage(shardOp{
        .valueType = searchFeatureReqType,
        .valuePtr = std::shared_ptr<searchFeatureReq>(new searchFeatureReq{query, topk}),
    });
    _channel->enqueueNotification(msg);
    auto output = msg->waitResponse();
    auto value = std::static_pointer_cast<searchFeatureRsp>(output.valuePtr);

    return std::move(value->fts);
};

RetCode ShardImpl::Close() {
    if (!HasWorker()) {
        spdlog::error("shard has no worker, AssignWorker first.");
        return RetCode::RET_ERR;
    }

    auto msg = NewWorkMessage(shardOp{closeShardReqType});
    _channel->enqueueNotification(msg);
    auto output = msg->waitResponse();
    auto value = std::static_pointer_cast<closeShardRsp>(output.valuePtr);

    _shard_info.is_closed = true;

    Stop();

    return RetCode::RET_OK;
};

void ShardImpl::loop() {
    for (;;) {
        if (_is_stopped.load()) {
            spdlog::info("get stopped flag, break loop");
            break;
        }
        // output is a blocking call.
        Notification::Ptr pNf = _channel->waitDequeueNotification(WAIT_MSG_INTERVAL_MS);
        if (pNf.isNull()) {
            spdlog::info("get null message, continue loop");
            continue;
        }

        WorkMessage<shardOp>::Ptr msg = pNf.cast<WorkMessage<shardOp>>();
        auto input = msg->getRequest();

        switch (input.valueType) {
        case assignWorkerReqType: {
            // start a fiber?
            {
                auto output = do_assign_worker(input);
                msg->setResponse(output);
            }
            break;
        }
        case addFeaturesReqType: {
            auto output = do_add_features(input);
            msg->setResponse(output);
            break;
        }
        case searchFeatureReqType: {
            auto output = do_search_feature(input);
            msg->setResponse(output);
            break;
        }
        case closeShardReqType: {
            auto output = do_close_shard(input);
            msg->setResponse(output);
            break;
        }
        default:
            spdlog::error("shard loop input value is not valid! wrong valueType: ");
            continue;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Inner implements.
////////////////////////////////////////////////////////////////////////////////////////////////////////////

shardOp ShardImpl::do_assign_worker(const shardOp& input) {
    auto req = std::static_pointer_cast<assignWorkerReq>(input.valuePtr);
    auto rsp = std::make_shared<assignWorkerRsp>();

    shardOp output{
        .valueType = addFeaturesRspType,
        .valuePtr = rsp,
    };
    return output;
};
shardOp ShardImpl::do_add_features(const shardOp& input) {
    auto req = std::static_pointer_cast<addFeaturesReq>(input.valuePtr);
    auto rsp = std::make_shared<addFeaturesRsp>();

    // delegate to worker.
    auto ret = _worker->AddFeatures(_db_id, _shard_id, req->fts);
    if (ret.size() > 0) {
        _shard_info.used += req->fts.size();
        _shard_mgr->UpdateShard(_shard_info);
        rsp->feature_ids = ret;
    }

    // TODO: what about _worker api error?

    shardOp output{
        .valueType = addFeaturesRspType,
        .valuePtr = rsp,
    };
    return output;
};

shardOp ShardImpl::do_search_feature(const shardOp& input) {
    auto req = std::static_pointer_cast<searchFeatureReq>(input.valuePtr);
    auto rsp = std::make_shared<searchFeatureRsp>();

    auto ret = _worker->SearchFeature(_shard_info.db_id, req->query, req->topk);
    rsp->fts.swap(ret);

    // TODO: what about _worker api error?

    shardOp output{
        .valueType = searchFeatureRspType,
        .valuePtr = rsp,
    };
    return output;
};

shardOp ShardImpl::do_close_shard(const shardOp& input) {
    auto req = std::static_pointer_cast<closeShardReq>(input.valuePtr);
    auto rsp = std::make_shared<closeShardRsp>();

    _worker->CloseShard(_db_id, _shard_id);
    _shard_mgr->CloseShard(_db_id, _shard_id);

    // TODO: what about _worker api error?

    shardOp output{
        .valueType = closeShardRspType,
        .valuePtr = rsp,
    };
    return output;
};

Shard* ShardFactoryImpl::CreateShard(ShardManager* mgr, DBShard shard_info) {
    return new ShardImpl(mgr, shard_info);
};

} // namespace donde_toolkits::feature_search::search_manager
