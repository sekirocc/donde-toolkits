#pragma once

#include "Poco/Format.h"
#include "Poco/Logger.h"
#include "Poco/Timestamp.h"
#include "config.h"
#include "gen/pb-cpp/feature_search_inner.grpc.pb.h"
#include "search/searcher.h"

// #include "spdlog/spdlog.h"

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <iostream>

using namespace std;

using com::sekirocc::feature_search::inner::BatchAddFeaturesRequest;
using com::sekirocc::feature_search::inner::BatchAddFeaturesResponse;
using com::sekirocc::feature_search::inner::BatchDeleteFeaturesRequest;
using com::sekirocc::feature_search::inner::BatchDeleteFeaturesResponse;
using com::sekirocc::feature_search::inner::FeatureSearch;
using com::sekirocc::feature_search::inner::SearchFeatureRequest;
using com::sekirocc::feature_search::inner::SearchFeatureResponse;
using com::sekirocc::feature_search::inner::TrainIndexRequest;
using com::sekirocc::feature_search::inner::TrainIndexResponse;

using grpc::ServerContext;
using grpc::Status;

class FeatureSearchWorkerImpl final : public FeatureSearch::Service {
  public:
    FeatureSearchWorkerImpl(Config& server_config);
    ~FeatureSearchWorkerImpl();

    void Start();
    void Stop();

    Status TrainIndex(ServerContext* context, const TrainIndexRequest* request,
                      TrainIndexResponse* response) override;

    Status BatchAddFeatures(ServerContext* context, const BatchAddFeaturesRequest* request,
                            BatchAddFeaturesResponse* response) override;

    Status BatchDeleteFeatures(ServerContext* context, const BatchDeleteFeaturesRequest* request,
                               BatchDeleteFeaturesResponse* response) override;

    Status SearchFeature(ServerContext* context, const SearchFeatureRequest* request,
                         SearchFeatureResponse* response) override;

  private:
    Config& config;
    std::shared_ptr<search::Searcher> searcher;
    // spdlog::Logger& logger;
};
