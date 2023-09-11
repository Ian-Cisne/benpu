#ifndef VIDEO_MANAGER_H
#define VIDEO_MANAGER_H

#include "configuration.h"
#include "status_code.h"
#include "window.h"
#include <cstddef>
#include <optional>
#include <vulkan/vulkan_handles.hpp>


namespace benpu {

class VideoManager {

 public:   
   VideoManager(): window(800, 600) {}

  ~VideoManager() {} ;

  void setUp();

  //In the future, this won't be here.
  void run();

  void dismantle();

private:
  Window window;
  vk::Instance instance = nullptr;
  vk::PhysicalDevice physicalDevice = nullptr;
  vk::Device device = nullptr;
  vk::Queue queue = nullptr;
  vk::Queue presentationQueue = nullptr;
  vk::SurfaceKHR surface = nullptr;

private:


  struct QueueFamilyIndices{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete () {return graphicsFamily.has_value() && presentFamily.has_value();}

  };

  StatusCode createInstance();
  StatusCode createSurface();
  StatusCode pickPhysicalDevice(QueueFamilyIndices& queueFamilyIndices);
  StatusCode createDevice(QueueFamilyIndices& queueFamilyIndices);
  int getBestPhysicalDevice(const std::vector<vk::PhysicalDevice>& physicalDevices, VideoManager::QueueFamilyIndices& queueFamilyIndices);
  
};

StatusCode checkRequiredExtensions(const std::vector<const char*> &requiredExtensions);
StatusCode checkRequiredLayers(const std::vector<const char*> &requiredLayers);

extern VideoManager mVideoManager;

} // namespace benpu
#endif
