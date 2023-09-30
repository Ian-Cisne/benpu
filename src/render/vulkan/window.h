#ifndef BENPU_WINDOW_H_
#define BENPU_WINDOW_H_

#include <tuple>
#include <vector>
#include "vkfw.hpp"

#include "core/utils/configuration.h"
#include "status_code.h"

namespace benpu {

class Window {

public:   
  Window(uint32_t width, uint32_t height);
  ~Window();

  bool shouldClose() const { return window.shouldClose(); };
  void pollEvents() { vkfw::pollEvents(); }
  vk::SurfaceKHR createSurface(vk::Instance& instance);
  std::tuple<int, int> getFramebufferSize() const;

  static std::vector<const char*> getRequiredVulkanExtensions();
  
private:
  uint32_t width;
  uint32_t height;
  vkfw::Window window;
  
};

} // namespace benpu
#endif
