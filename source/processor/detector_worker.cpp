#include "Poco/AutoPtr.h"
#include "Poco/Logger.h"
#include "Poco/Notification.h"
#include "Poco/NotificationQueue.h"
#include "concurrent_processor.h"
#include "detector_worker.h"
#include "types.h"
#include "utils.h"

#include "opencv2/opencv.hpp"

#include "openvino/openvino.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <cassert>

using Poco::Logger;
using Poco::Notification;
using Poco::NotificationQueue;

using namespace Poco;

DetectorWorker::DetectorWorker(std::shared_ptr<NotificationQueue> ch) : Worker(ch) {}

DetectorWorker::~DetectorWorker() {
    // _channel.reset();
}

/**

   conf:
   {
     "model": "../models/face-detection-adas-0001.xml"
   }

 */


void printInputTensor(const ov::Tensor& input) {
    ov::Shape shape = input.get_shape();
    const size_t batch_size = shape[0];
    using tensor_type = ov::fundamental_type_for<ov::element::Type_t::f32>;
    const tensor_type* tensor_data = input.data<const tensor_type>();

    
    
    
}


RetCode DetectorWorker::Init(json conf, int i, std::string device_id) {
    _id = i;
    _name = "detector-worker-" + std::to_string(i);
    _device_id = device_id;
    _conf = conf;

    std::string model_path = conf["model"];

    std::cout << "loading model: " << model_path << std::endl;
    std::cout << "abs path: " << std::filesystem::canonical(model_path) << std::endl;

    ov::Core core;
    std::shared_ptr<ov::Model> model = core.read_model(model_path);
    printInputAndOutputsInfo(*model);

    OPENVINO_ASSERT(model->inputs().size() == 1, "Sample supports models with 1 input only");
    OPENVINO_ASSERT(model->outputs().size() == 1, "Sample supports models with 1 output only");

    ov::preprocess::PrePostProcessor ppp(model);
    ov::preprocess::InputInfo& input_info = ppp.input();

    // input data is NHWC
    ov::Layout tensor_layout{"NHWC"};
    input_info.tensor().set_element_type(ov::element::u8).set_layout(tensor_layout);
    // but the model input need nchw, so the convertion begin.
    input_info.model().set_layout("NCHW");

    // model output is float32
    ppp.output().tensor().set_element_type(ov::element::f32);

    _model = ppp.build();
    
    ov::Shape input_shape = _model->input().get_shape();
    const size_t width = input_shape[ov::layout::width_idx(tensor_layout)];
    const size_t height = input_shape[ov::layout::height_idx(tensor_layout)];

    std::string warmup_image = "/Users/jiechen/Documents/Images/white_background.jpg";

    cv::Mat img = cv::imread(warmup_image);

    printf("resize image from [%d x %d] to [%d x %d]", img.cols, img.rows, (int)width, (int)height);

    const size_t data_length = img.channels() * width * height;
    std::shared_ptr<unsigned char> data_ptr;
    data_ptr.reset(new unsigned char[data_length], std::default_delete<unsigned char[]>());

    cv::Size new_size(width, height);
    cv::Mat resized_img(new_size, img.type(), data_ptr.get());
    cv::resize(img, resized_img, new_size);
    
    ov::CompiledModel compiled_model = core.compile_model(_model, "CPU");
    ov::InferRequest infer_request = compiled_model.create_infer_request();
    ov::Tensor input_tensor = infer_request.get_input_tensor();
    
    const size_t batch_size = 1;
    ov::set_batch(_model, batch_size);

    const size_t image_size = ov::shape_size(model->input().get_shape()) / batch_size;
    assert(image_size == data_length);
    
    std::memcpy(input_tensor.data<std::uint8_t>(),
                data_ptr.get(),
                image_size);
    
    infer_request.infer();
    
    const ov::Tensor& output_tensor = infer_request.get_output_tensor();
    
    printOutputTensor(output_tensor);

    return RET_OK;
}

void DetectorWorker::run() {
    for (;;) {
        Notification::Ptr pNf(_channel->waitDequeueNotification());

        if (pNf) {
            WorkMessage::Ptr msg = pNf.cast<WorkMessage>();
            if (msg) {
                if (msg->isQuitMessage()) {
                    break;
                }
                Value input = msg->getRequest();
                DetectResult result;
                Value output{ValueDetectResult, (void*)&result};
                msg->setResponse(output);
            }
        } else {
            break;
        }
    }

}
