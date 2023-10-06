#pragma once

#include <cstdint>

namespace donde_toolkits ::video_process {

struct ProcessOptions {
    int warm_up_frames;
    int skip_frames;
    int decode_fps;
    bool loop_forever;
};

struct VideoStreamInfo {
    bool open_success;
    int64_t nb_frames;
    int64_t duration_s;
};

class VideoProcessor {
  public:
    virtual VideoStreamInfo Process(const ProcessOptions& opts) = 0;

    virtual bool Pause() = 0;

    virtual bool Resume() = 0;

    virtual bool Stop() = 0;
};


} // namespace donde_toolkits::video_process
