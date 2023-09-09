#ifndef VIDEO_MANAGER_H
#define VIDEO_MANAGER_H

#include <vulkan/vulkan.h>

#include "configuration.h"
#include "status_code.h"

namespace benpu {

class VideoManager {

 public:   
   VideoManager() {}

  ~VideoManager() {}

  void setUp();

  void dismantle();

private:
  dynamic_libray vulkanLibrary;
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

private:
  StatusCode loadVulkanLibrary();
  StatusCode loadExportedVulkanFunction();
  StatusCode loadGlobalVulkanFunction();
};

extern VideoManager mVideoManager;

} // namespace benpu
#endif
