#ifndef WINDOW_H
#define WINDOW_H

#include <tuple>
#include <vector>
#include "vkfw.hpp"
#include <vulkan/vulkan_handles.hpp>


#include "core/utils/configuration.h"
#include "status_code.h"

namespace benpu {

class Window {

 public:   
   Window(uint32_t width, uint32_t height);

  ~Window();

  bool shouldClose() { return window.shouldClose(); };
  void pollEvents() { vkfw::pollEvents(); }
  std::vector<const char*> getRequiredVulkanExtensions();
  vk::SurfaceKHR createSurface(vk::Instance& instance);
  std::tuple<int, int> getFramebufferSize();

private:
  uint32_t width;
  uint32_t height;
  vkfw::Window window; 
  
};

} // namespace benpu
#endif
