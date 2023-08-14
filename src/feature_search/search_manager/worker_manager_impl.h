#pragma once

#include "donde/definitions.h"
#include "donde/feature_search/definitions.h"
#include "donde/feature_search/driver.h"
#include "donde/feature_search/search_manager/worker_manager.h"
#include "donde/feature_search/worker.h"

#include <iostream>
#include <memory>
#include <unordered_map>

using namespace std;

namespace donde_toolkits ::feature_search ::search_manager {

class WorkerManagerImpl : public IWorkerManager {
  public:
    WorkerManagerImpl(Driver& driver);
    ~WorkerManagerImpl();

    void Stop() override;

    Worker* FindWritableWorker() override;

    // AttachWorker pass in a worker poiter, then we own this worker
    void AttachNewWorker(Worker* worker) override;
    bool AllWorkersOnline() override;

    // Connect to these workers, attach them in our online_workers if they are online
    // or we can only wait for workers to come up...
    void LoadKnownWorkers() override;

  private:
    void ping_workers();

  private:
    Driver& _driver;

    // id -> Worker*
    std::unordered_map<std::string, Worker*> _online_workers;
    // id -> address
    std::unordered_map<std::string, WorkerItem> _offline_workers;

    std::thread _ping_thread;
    bool stopped = false;
};

} // namespace donde_toolkits::feature_search::search_manager
