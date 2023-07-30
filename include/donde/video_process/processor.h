#pragma once

namespace donde_toolkits ::video_process {

class VideoProcessor {
  public:
    virtual bool Process() = 0;

    virtual bool Pause() = 0;

    virtual bool Resume() = 0;

    virtual bool Stop() = 0;
};

} // namespace donde_toolkits::video_process
