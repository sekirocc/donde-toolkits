#include "donde/definitions.h"
#include "donde/feature_search/definitions.h"
#include "donde/feature_search/search_worker/brute_force_worker.h"
#include "donde/feature_search/simple_driver.h"
#include "donde/utils.h"
#include "src/feature_search/search_worker/shard.h"
#include "src/feature_search/search_worker/shard_manager_impl.h"

#include <cstdlib>
#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <ostream>



using nlohmann::json;

using donde_toolkits::feature_search::DBItem;
using donde_toolkits::feature_search::FeatureDbItem;
using donde_toolkits::feature_search::FeatureSearchItem;
using donde_toolkits::feature_search::SimpleDriver;

using donde_toolkits::feature_search::search_worker::BruteForceWorker;
using donde_toolkits::feature_search::search_worker::ShardManager;
using donde_toolkits::feature_search::search_worker::ShardManagerImpl;

using donde_toolkits::Feature;
using donde_toolkits::gen_feature_dim;

namespace {

class SearchManager_BruteForceSearch : public ::testing::Test {
  protected:
    void SetUp() override {
        for (int i = 0; i < feature_count; i++) {
            auto ft = gen_feature_dim<512>();
            std::map<std::string, std::string> meta{{"keya", "valueb"}};
            fts.push_back(FeatureDbItem{
                .feature = ft,
                .metadata = meta,
            });
        }

        store = new SimpleDriver("/tmp/test_store");
        DBItem db1{
            .name = "test-db1",
            .size = 1024,
            .description = "this is a test db",
        };

        db_id = store->CreateDB(db1);
    };

    void TearDown() override {
        // cleanup db file
        std::filesystem::remove_all("/tmp/test_store/");
    };

    SimpleDriver* store;
    const int feature_count = 100;
    const int dim = 512;
    std::vector<FeatureDbItem> fts;
    std::string db_id;
};

TEST_F(SearchManager_BruteForceSearch, BruteForceSearch) {
    BruteForceWorker search(*store);

    std::vector<std::string> feature_ids = search.AddFeatures(db_id, fts);
    EXPECT_EQ(feature_ids.size(), feature_count);
}

TEST_F(SearchManager_BruteForceSearch, SearchTopkTeatures) {
    BruteForceWorker search(*store);

    // preapre features in db.
    std::vector<std::string> feature_ids = search.AddFeatures(db_id, fts);
    EXPECT_EQ(feature_ids.size(), feature_count);

    // query is the first one. so that we definitely can find matched fts.
    Feature query{fts[0].feature};

    int topk = 10;
    std::vector<FeatureSearchItem> search_result = search.SearchFeature(db_id, query, topk);

    for (const auto& r : search_result) {
        std::cout << "score: " << r.score << std::endl;
    }

    EXPECT_EQ(search_result.size(), topk);

    if (int(search_result.size()) == topk) {
        // the most near ft.
        auto t = search_result[0];
        EXPECT_GT(t.score, 0.99);

        // query.debugPrint();
        // t.target.debugPrint();

        for (size_t i = 0; i < t.target.raw.size(); i++) {
            if (t.target.raw[i] != query.raw[i]) {
                std::cout << "i : " << i << std::endl;
            }
            EXPECT_EQ(t.target.raw[i], query.raw[i]);
        }
    }
}

} // namespace
