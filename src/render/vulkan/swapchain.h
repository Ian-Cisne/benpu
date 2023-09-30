#ifndef BENPU_SWAPCHAIN_H_
#define BENPU_SWAPCHAIN_H_

#include <memory>

#include <vulkan/vulkan.hpp>

#include "render/vulkan/queue.h"
#include "render/vulkan/render_pass.h"
#include "status_code.h"

namespace benpu {

struct SwapChainSupportDetails {
  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;
};

class Swapchain {
public:
  Swapchain(vk::Device& device);

  StatusCode setSurface(vk::Instance &instance, Window &window);

  StatusCode initialize(vk::PhysicalDevice& physicalDevice, const QueueFamilyIndices& queueFamilyIndices, const Window& window);
  StatusCode createFramebuffers(const RenderPass& renderPass);

  vk::SurfaceKHR getSurface() const;
  vk::Format getFormat() const;

  SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice physicalDevice);

private:

  vk::Device& device;
  vk::SurfaceKHR surface = nullptr;
  vk::SwapchainKHR swapchain = nullptr;
  std::vector<vk::Image> swapChainImages;
  vk::Format imageFormat;
  vk::Extent2D extent;
  std::vector<vk::ImageView> swapChainImageViews;
  std::vector<vk::Framebuffer> swapChainFramebuffers;
  vk::Semaphore imageAvailableSemaphore = nullptr;
  vk::Semaphore renderFinishedSemaphore = nullptr;
  Queue presentationQueue;
  Queue graphicsQueue;
  SwapChainSupportDetails supportDetails;
  

private:
  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
  vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const Window& window);
  StatusCode createImageViews();
  
};

} //namespace benpu

#endif