#pragma once

#include "Poco/Event.h"
#include "Poco/Logger.h"
#include "Poco/Notification.h"
#include "Poco/NotificationQueue.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "Poco/ThreadPool.h"
#include "concurrent_processor.h"
#include "types.h"
#include "nlohmann/json.hpp"

#include <iostream>
#include <memory>

using Poco::AutoPtr;
using Poco::FastMutex;
using Poco::Logger;
using Poco::Notification;
using Poco::NotificationQueue;
using Poco::Runnable;
using Poco::ThreadPool;

using json = nlohmann::json;

using namespace Poco;

class DetectorWorker : public Worker {
  public:
    DetectorWorker(std::shared_ptr<NotificationQueue> ch);
    ~DetectorWorker();

    RetCode Init(json conf, int id, std::string device_id) override;

    void run() override;

  private:
    Logger& _logger = Logger::get("DetectorLogger");
};

// class LandmarksWorker : public Worker {
//   public:
//     LandmarksWorker(std::shared_ptr<NotificationQueue> ch);
//     ~LandmarksWorker();
//
//     RetCode Init(json conf, int id, std::string device_id) override;
//
//     void run() override;
//
//   private:
//     Poco::Logger& _logger;
// };
//
// class AlignerWorker : public Worker {
//   public:
//     AlignerWorker(std::shared_ptr<NotificationQueue> ch);
//     ~AlignerWorker();
//
//     RetCode Init(json conf, int id, std::string device_id) override;
//
//     void run() override;
//
//   private:
//     Poco::Logger& _logger;
// };
//
// class FeatureWorker : public Worker {
//   public:
//     FeatureWorker(std::shared_ptr<NotificationQueue> ch);
//     ~FeatureWorker();
//
//     RetCode Init(json conf, int id, std::string device_id) override;
//
//     void run() override;
//
//   private:
//     Poco::Logger& _logger;
// };
//
