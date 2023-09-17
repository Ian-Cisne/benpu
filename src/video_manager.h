#ifndef VIDEO_MANAGER_H
#define VIDEO_MANAGER_H

#include "configuration.h"
#include "status_code.h"
#include "window.h"
#include <cstddef>
#include <optional>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>


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
  vk::SwapchainKHR swapchain = nullptr;
  std::vector<vk::Image> swapChainImages;
  vk::Format swapChainImageFormat;
  vk::Extent2D swapChainExtent;
  std::vector<vk::ImageView> swapChainImageViews;
  vk::ShaderModule fragShaderModule = nullptr;
  vk::ShaderModule vertShaderModule = nullptr;
  std::vector<vk::DynamicState> dynamicStates = {
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor
  };
  vk::PipelineLayout pipelineLayout = nullptr;
  vk::RenderPass renderPass = nullptr;
  vk::Pipeline graphicsPipeline = nullptr;
  std::vector<vk::Framebuffer> swapChainFramebuffers;
  vk::CommandPool commandPool;
  vk::CommandBuffer commandBuffer;

private:


  struct QueueFamilyIndices{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete () {return graphicsFamily.has_value() && presentFamily.has_value();}

  };

  struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
  };

  StatusCode createInstance();
  StatusCode createSurface();
  StatusCode pickPhysicalDevice(QueueFamilyIndices& queueFamilyIndices, const std::vector<const char*>& requiredExtensions);
  StatusCode createDevice(QueueFamilyIndices& queueFamilyIndices, const std::vector<const char*>& requiredExtensions);
  SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);
  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
  vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
  StatusCode createSwapChain(QueueFamilyIndices& queueFamilyIndices);
  StatusCode createImageViews();
  StatusCode createGraphicsPipeline();
  StatusCode createShaderModule(const std::vector<char>& code, vk::ShaderModule& shaderModule);
  StatusCode createRenderPass();
  StatusCode createFramebuffers();
  StatusCode createCommandPool(QueueFamilyIndices& queueFamilyIndices);
  StatusCode createCommandBuffer();

  int getBestPhysicalDevice(const std::vector<vk::PhysicalDevice>& physicalDevices, VideoManager::QueueFamilyIndices& queueFamilyIndices, const std::vector<const char*>& requiredExtensions);
  
};

StatusCode checkRequiredExtensions(const std::vector<const char*> &requiredExtensions);
StatusCode checkRequiredLayers(const std::vector<const char*> &requiredLayers);

extern VideoManager mVideoManager;

} // namespace benpu
#endif
