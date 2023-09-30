
#include <boost/log/trivial.hpp>

#include "vkfw.hpp"

#include "render/vulkan/window.h"

namespace benpu {

  Window::Window(uint32_t width, uint32_t height): width{width}, height{height} {
    
    vkfw::WindowHints hints;
    hints.clientAPI = vkfw::ClientAPI::eNone;
    //for now it won't be resizable
    hints.resizable = false;

    window = vkfw::createWindow(width, height, "Benpu", hints);
  }

  Window::~Window() {
    window.destroy();
  }

  std::vector<const char*> Window::getRequiredVulkanExtensions() {
    
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = vkfw::getRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector< const char*> vector(glfwExtensionCount);

    for (size_t i = 0; i < glfwExtensionCount; ++i) {
        vector[i] = glfwExtensions[i];
    }

    return vector;
  }

  std::tuple<int, int> Window::getFramebufferSize() const {
    return window.getFramebufferSize();
  }

  vk::SurfaceKHR Window::createSurface(vk::Instance& instance) {
    return vkfw::createWindowSurface(instance, window);
  }

} // namespace benpu