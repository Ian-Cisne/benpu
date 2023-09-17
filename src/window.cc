
#include "window.h"
#include "vkfw.hpp"

namespace benpu {

  Window::Window(uint32_t width, uint32_t height): width{width}, height(height){
    vkfw::init();

    vkfw::WindowHints hints;
    hints.clientAPI = vkfw::ClientAPI::eNone;
    hints.transparentFramebuffer = true;
    //for now it won't be resizable
    hints.resizable = false;
    hints.alphaBits = true;
    hints.decorated = false;
    window = vkfw::createWindow(width, height, "Benpu", hints);
  }

  Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  std::vector<const char*> Window::getRequiredVulkanExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = vkfw::getRequiredInstanceExtensions(&glfwExtensionCount);//glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector< const char*> vector(glfwExtensionCount);

    for (size_t i = 0; i < glfwExtensionCount; ++i) {
        vector[i] = glfwExtensions[i];
    }
    return vector;
  }

  std::tuple<int, int> Window::getFramebufferSize() {
    return window.getFramebufferSize();
  }

  vk::SurfaceKHR Window::createSurface(vk::Instance& instance) {
    return vkfw::createWindowSurface(instance, window);
  }

} // namespace benpu