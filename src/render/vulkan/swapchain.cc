
#include <boost/log/trivial.hpp>

#include "render/vulkan/window.h"
#include "render/vulkan/swapchain.h"
#include "status_code.h"

namespace benpu {

Swapchain::Swapchain(vk::Device& device): 
  device{device},
  graphicsQueue{device},
  presentationQueue{device} {

}

vk::SurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
  for (const auto& availableFormat : availableFormats) {
    if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return availableFormat;
    }
  }
  return availableFormats[0];
}

vk::PresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
  for (const auto& availablePresentMode : availablePresentModes) {
    if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
      return availablePresentMode;
    }
  }
  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const Window& window){
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    auto [width, height] = window.getFramebufferSize();

    vk::Extent2D actualExtent = {static_cast<uint32_t>(width),
                                static_cast<uint32_t>(height)};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                    capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                    capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

StatusCode Swapchain::initialize(vk::PhysicalDevice& physicalDevice, const QueueFamilyIndices& queueFamilyIndices, const Window& window) {
  try {
    
    if(graphicsQueue.initialize(queueFamilyIndices.graphicsFamily.value()) != StatusCode::success) {
      BOOST_LOG_TRIVIAL(error) << "Couldn't create graphics queue.";
      return StatusCode::queueCreationError;
    }
    
    if(presentationQueue.initialize(queueFamilyIndices.presentFamily.value()) != StatusCode::success) {
      BOOST_LOG_TRIVIAL(error) << "Couldn't create presentiation queue.";
      return StatusCode::queueCreationError;
    }

    BOOST_LOG_TRIVIAL(info) << "Creating swapchain.";

    querySwapChainSupport(physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(supportDetails.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(supportDetails.presentModes);
    extent = chooseSwapExtent(supportDetails.capabilities, window);

    uint32_t imageCount = supportDetails.capabilities.minImageCount;
    if (supportDetails.capabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount) {
      imageCount = supportDetails.capabilities.maxImageCount;
    }

    uint32_t queueFamilyIndicesArray[] = {queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value()};

    vk::SwapchainCreateInfoKHR createInfo(
      {},
      surface,
      imageCount,
      surfaceFormat.format,
      surfaceFormat.colorSpace,
      extent,
      1,
      vk::ImageUsageFlagBits::eColorAttachment,
      vk::SharingMode::eConcurrent,
      1,
      queueFamilyIndicesArray,
      supportDetails.capabilities.currentTransform,
      vk::CompositeAlphaFlagBitsKHR::eInherit,
      presentMode,
      vk::True
    );

    if (queueFamilyIndices.graphicsFamily == queueFamilyIndices.presentFamily) {
      createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
      createInfo.setQueueFamilyIndexCount(2);
      createInfo.setQueueFamilyIndices(queueFamilyIndicesArray);
    }

    swapchain = device.createSwapchainKHR(createInfo);
    swapChainImages = device.getSwapchainImagesKHR(swapchain);
    imageFormat = surfaceFormat.format;

  } catch (vk::SystemError& e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while Swapchain creation: " << e.what();
    return StatusCode::swapchainCreationError;

  }
  
  if(createImageViews() != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create image views.";
    return StatusCode::imageViewsCreationError;
  }

  return StatusCode::success;
}

StatusCode Swapchain::createImageViews() {
  try {

    BOOST_LOG_TRIVIAL(info) << "Creating image views.";

    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
      vk::ImageViewCreateInfo createInfo(
        {},
        swapChainImages[i],
        vk::ImageViewType::e2D,
        imageFormat,
        vk::ComponentMapping(
            vk::ComponentSwizzle::eIdentity,
          vk::ComponentSwizzle::eIdentity,
          vk::ComponentSwizzle::eIdentity,
          vk::ComponentSwizzle::eA
          ),
        vk::ImageSubresourceRange(
            vk::ImageAspectFlagBits::eColor,
            0,
            1,
            0,
            1
          )
      );
      swapChainImageViews[i] = device.createImageView(createInfo);
    }

  } catch(vk::SystemError& e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while image views creation: " << e.what();
    return StatusCode::imageViewsCreationError;
  }
  return StatusCode::success;
}

StatusCode Swapchain::createFramebuffers(const RenderPass& renderPass) {

  BOOST_LOG_TRIVIAL(info) << "Creating framebuffers.";

  swapChainFramebuffers.resize(swapChainImageViews.size());

  try {
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
      vk::ImageView attachments[] = {
        swapChainImageViews[i]
      };

      vk::FramebufferCreateInfo framebufferInfo(
        {},
        renderPass.getRenderPass(),
        1,
        attachments,
        extent.width,
        extent.height,
        1

      );
      
      if (device.createFramebuffer(&framebufferInfo, nullptr, &swapChainFramebuffers[i]) != vk::Result::eSuccess) {
        BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while framebuffer number " + std::to_string(i) + ".";
        return StatusCode::frameBufferCreationError;
      }
    }

  } catch (vk::SystemError& e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while framebuffers creation: " << e.what();
    return StatusCode::frameBufferCreationError;
  }

  return StatusCode::success;
}

StatusCode Swapchain::setSurface(vk::Instance& instance, Window& window) {
  try {
    surface = window.createSurface(instance);
  } catch (const vk::SystemError& e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while surface creation: " << e.what();
    return StatusCode::vulkanError;
  }
  return StatusCode::success;
}

vk::SurfaceKHR Swapchain::getSurface() const {
  return surface;
}

vk::Format Swapchain::getFormat() const {
  return imageFormat;
}

SwapChainSupportDetails Swapchain::querySwapChainSupport(vk::PhysicalDevice physicalDevice) {

  supportDetails.formats = physicalDevice.getSurfaceFormatsKHR(surface);
  supportDetails.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
  supportDetails.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

  return supportDetails;

}

} //namespace benpu