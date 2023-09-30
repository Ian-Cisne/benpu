#ifndef BENPU_RENDERER_H_
#define BENPU_RENDERER_H_

#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "render/vulkan/command_pool.h"
#include "render/vulkan/window.h"
#include "render/vulkan/swapchain.h"
#include "render/vulkan/pipeline.h"
#include "render/vulkan/queue.h"
#include "render/vulkan/render_pass.h"

namespace benpu {

class Renderer {
public:
  static Renderer& getInstance();
  void mainLoop();

  ObjectStatus getStatus() { return status; }

private:
  Renderer();
  ~Renderer();
  Window mainWindow;
  vk::Instance instance = nullptr;
  vk::PhysicalDevice physicalDevice = nullptr;
  vk::Device device = nullptr;
  Swapchain swapchain;
  Pipeline pipeline;
  RenderPass renderPass;
  CommandPool commandPool;

  ObjectStatus status = unitialized;


private:

  StatusCode createVulkanInstance();
  StatusCode pickPhysicalDevice(QueueFamilyIndices& queueFamilyIndices, const std::vector<const char*>& requiredExtensions);
  StatusCode createDevice(QueueFamilyIndices& queueFamilyIndices, const std::vector<const char*>& requiredExtensions);
  int getBestPhysicalDevice(const std::vector<vk::PhysicalDevice>& physicalDevices, QueueFamilyIndices& bestDeviceQueueFamilyIndices, const std::vector<const char*>& requiredExtensions);

  void drawFrame();
};

} //namespace benpu

#endif