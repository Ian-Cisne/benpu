#ifndef VIDEO_MANAGER_H
#define VIDEO_MANAGER_H

namespace benpu {

class VideoManager {
  VideoManager() {}
  ~VideoManager() {}
  
  void setUp();

  void dismantle();
};

extern VideoManager mVideoManager;

} // namespace benpu
#endif
