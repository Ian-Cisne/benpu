#include "video_manager.h"

#include <algorithm>

#include <boost/log/core.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/trivial.hpp>
#include <cstddef>
#include <fstream>
#include <memory>
#include <set>
#include <string>
#include <sys/types.h>
#include <tuple>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "status_code.h"

namespace benpu {

void VideoManager::setUp() {

  if(createInstance() != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create Vulkan instance.";
  }

  if(createSurface() != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create surface.";
  }

  std::vector<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
  QueueFamilyIndices queueFamilyIndices;
  if(pickPhysicalDevice(queueFamilyIndices, requiredExtensions) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't pick a physical device.";
  }

  if(createDevice(queueFamilyIndices, requiredExtensions) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create logical device.";
  }
  
  if(createSwapChain(queueFamilyIndices) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create swapchain.";
  }
  
  if(createImageViews() != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create image views.";
  }

  if(createGraphicsPipeline() != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create graphics pipeline.";
  }

  if(createRenderPass() != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create render pass.";
  }

}

void VideoManager::run() {
  while (!window.shouldClose()) {
    window.pollEvents();
  }
}

StatusCode VideoManager::createInstance() {
  try {
    BOOST_LOG_TRIVIAL(info) << "Creating application info.";

    vk::ApplicationInfo appInfo(
      "Benpu",
      VK_MAKE_API_VERSION(0, 0, 1, 0),
      "Benpu engine",
      VK_MAKE_API_VERSION(0, 0, 1, 0),
      VK_API_VERSION_1_3);

    BOOST_LOG_TRIVIAL(info) << "Created application info.";

    std::vector<const char *> requiredExtensions = window.getRequiredVulkanExtensions();

    requiredExtensions.push_back("VK_KHR_portability_enumeration");

    if (checkRequiredExtensions(requiredExtensions) != StatusCode::success) {
      BOOST_LOG_TRIVIAL(error) << "One or more of required extensions haven't been found.";

    }

    std::vector<const char *> requiredLayers{"VK_LAYER_KHRONOS_validation"};

    BOOST_LOG_TRIVIAL(info) << "Checking required layers.";

    if (checkRequiredLayers(requiredLayers) != StatusCode::success) {
      BOOST_LOG_TRIVIAL(error) << "One or more of required layers haven't been found.";

    }

    BOOST_LOG_TRIVIAL(info) << "Creating instance info.";
    vk::InstanceCreateInfo instanceInfo(
        vk::InstanceCreateFlags{VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR | VK_KHR_portability_enumeration}, 
        &appInfo,
        static_cast<uint32_t>(requiredLayers.size()),
        requiredLayers.data(), static_cast<uint32_t>(requiredExtensions.size()),
        requiredExtensions.data());

    BOOST_LOG_TRIVIAL(info) << "Creating instance.";
    instance = vk::createInstance(instanceInfo);

  } catch (vk::SystemError &e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while Instance creation: " << e.what();
    return StatusCode::vulkanError;
  }
  return StatusCode::success;
}

StatusCode VideoManager::createSurface() {
  try {
    surface = window.createSurface(instance);
  } catch (const vk::SystemError& e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while surface creation: " << e.what();
    return StatusCode::vulkanError;
  }
  return StatusCode::success;
}

bool checkDeviceExtensionSupport(vk::PhysicalDevice device, const std::vector<const char*>& deviceExtensions) {
    std::vector<vk::ExtensionProperties> availableExtension = device.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const vk::ExtensionProperties& extension : availableExtension) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

VideoManager::SwapChainSupportDetails VideoManager::querySwapChainSupport(vk::PhysicalDevice device) {
  SwapChainSupportDetails details;
  details.formats = device.getSurfaceFormatsKHR(surface);
  details.presentModes = device.getSurfacePresentModesKHR(surface);
  details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
  return details;

}

vk::SurfaceFormatKHR VideoManager::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
  for (const auto& availableFormat : availableFormats) {
      if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
          return availableFormat;
      }
  }
  return availableFormats[0];
}

vk::PresentModeKHR VideoManager::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
  for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VideoManager::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities){
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

StatusCode VideoManager::createSwapChain(QueueFamilyIndices& queueFamilyIndices) {
  try {
    BOOST_LOG_TRIVIAL(info) << "Creating swapchain.";

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
      imageCount = swapChainSupport.capabilities.maxImageCount;
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
      queueFamilyIndices.graphicsFamily == queueFamilyIndices.presentFamily 
        ? vk::SharingMode::eExclusive
        : vk::SharingMode::eConcurrent,
      queueFamilyIndices.graphicsFamily == queueFamilyIndices.presentFamily
        ? 0
        : 2,
      queueFamilyIndices.graphicsFamily == queueFamilyIndices.presentFamily
        ? nullptr
        : queueFamilyIndicesArray,
        swapChainSupport.capabilities.currentTransform,
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
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

  } catch (vk::SystemError& e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while Swapchain creation: " << e.what();
    return StatusCode::swapchainreationError;

  }
  return StatusCode::success;
}

int VideoManager::getBestPhysicalDevice(const std::vector<vk::PhysicalDevice>& physicalDevices, VideoManager::QueueFamilyIndices& bestDeviceQueueFamilyIndices, const std::vector<const char*>& requiredExtensions) {

  int bestPhysicalDevicesIndex = -1;
  int bestScore = 0;


  for(int i = 0; i - physicalDevices.size(); ++i) {
    QueueFamilyIndices queueFamilyIndices;
    vk::PhysicalDeviceProperties deviceProperties = physicalDevices[i].getProperties();
    vk::PhysicalDeviceFeatures deviceFeatures = physicalDevices[i].getFeatures();
    
    std::vector<vk::QueueFamilyProperties> queueFamiliesProperties = physicalDevices[i].getQueueFamilyProperties();

    int score = 0;
    
    if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        score += 1000;
    } else if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
        score += 500;
    }
    
    uint32_t apiVersion = deviceProperties.apiVersion;
    if (VK_VERSION_MAJOR(apiVersion) >= 1 && VK_VERSION_MINOR(apiVersion) >= 2) {
        score += 500; 
    }
    
    uint32_t maxTextureSize = deviceProperties.limits.maxImageDimension2D;
    score += (maxTextureSize/4096) * 250;
    
    if (deviceFeatures.tessellationShader) {
        score += 100;
    }

    for (int j = 0; j < queueFamiliesProperties.size(); ++j) {
      
      if (queueFamiliesProperties[j].queueFlags & vk::QueueFlagBits::eGraphics) {
        queueFamilyIndices.graphicsFamily = j;
      }

      if (physicalDevices[i].getSurfaceSupportKHR(j, surface)) {
        queueFamilyIndices.presentFamily = j;
      }

      if (queueFamilyIndices.isComplete()) {
        break;
      }

    }
    
    if (!deviceFeatures.geometryShader
      || !queueFamilyIndices.isComplete()
      || !checkDeviceExtensionSupport(physicalDevices[i], requiredExtensions)) {
        //If it does support the queue families we need, we can't use it.
        score = 0;
        continue; 
    }

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevices[i]);

    if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) {
      score = 0;
    }
    
    if (score > bestScore) {
      bestScore = score;
      bestPhysicalDevicesIndex = i;
      bestDeviceQueueFamilyIndices = queueFamilyIndices;
      
    }
  }

  return bestPhysicalDevicesIndex;
}

StatusCode VideoManager::pickPhysicalDevice(QueueFamilyIndices &queueFamilyIndices, const std::vector<const char*>& requiredExtensions) {
  auto availablePhysicalDevices = instance.enumeratePhysicalDevices();

  if (availablePhysicalDevices.size() == 0) {
    return StatusCode::noPhysicalDeviceFound;
  }

  int bestPhysicalDeviceIndex;
  bestPhysicalDeviceIndex = getBestPhysicalDevice(availablePhysicalDevices, queueFamilyIndices, requiredExtensions);

  if (bestPhysicalDeviceIndex == -1) {
    return StatusCode::noProprerPhysicalDeviceFound;
  }

  physicalDevice = availablePhysicalDevices[bestPhysicalDeviceIndex];

  return StatusCode::success;
}

StatusCode checkRequiredExtensions(const std::vector<const char*> &requiredExtensions) {
  BOOST_LOG_TRIVIAL(info) << "Checking required Extensions.";
  
  std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();
  for (auto requiredExtension : requiredExtensions) {
    bool extensionFound = false;
    for (const vk::ExtensionProperties availableExtension : availableExtensions) {
      if (strcmp(requiredExtension, availableExtension.extensionName.data()) == 0) {
        extensionFound = true;
        break;
      }
    }
    if (!extensionFound) {
      return StatusCode::extensionNotFound;
    }
  }
  return StatusCode::success;
}

StatusCode checkRequiredLayers(const std::vector<const char*> &requiredLayers) {
  std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();
  for (const char* requiredLayer : requiredLayers) {
      bool layerFound = false;
    for (const vk::LayerProperties availableLayer : availableLayers) {
      if (strcmp(requiredLayer, availableLayer.layerName) == 0) {
        layerFound = true;
        break;
      }
    }
    if (!layerFound) {
      return StatusCode::layerNotFound;
    }
  }
  return StatusCode::success;
}

StatusCode VideoManager::createDevice(QueueFamilyIndices& queueFamilyIndices, const std::vector<const char*>& requiredExtensions) {
  try {

    BOOST_LOG_TRIVIAL(info) << "Creating logical device.";

    float queuePriority = 1.0f;

    vk::DeviceQueueCreateInfo queueInfo({},queueFamilyIndices.graphicsFamily.value(), 1, &queuePriority);
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {queueFamilyIndices.graphicsFamily.value(),
                                              queueFamilyIndices.presentFamily.value()};

    for (uint32_t queueFamily : uniqueQueueFamilies) {
      vk::DeviceQueueCreateInfo queueCreateInfo(
        {},
        queueFamily,
        1,
        &queuePriority
      );
      queueCreateInfos.push_back(queueCreateInfo);
    }
    vk::PhysicalDeviceFeatures deviceFeatures;

    vk::DeviceCreateInfo deviceInfo(
      {},
      static_cast<uint32_t>(queueCreateInfos.size()),
      queueCreateInfos.data(),
      {},
      {},
      static_cast<uint32_t>(requiredExtensions.size()),
      requiredExtensions.data(),
      &deviceFeatures
    );

    device = physicalDevice.createDevice(deviceInfo);
    
    device.getQueue(queueFamilyIndices.graphicsFamily.value(), 0, &queue);
    device.getQueue(queueFamilyIndices.presentFamily.value(), 0, &presentationQueue);

  } catch (vk::SystemError& e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while Device creation: " << e.what();
    return StatusCode::deviceCreationError;
  }
  return StatusCode::success;
}

StatusCode VideoManager::createImageViews(){
  try {

    BOOST_LOG_TRIVIAL(info) << "Creating image views.";

    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
      vk::ImageViewCreateInfo createInfo(
        {},
        swapChainImages[i],
        vk::ImageViewType::e2D,
        swapChainImageFormat,
        vk::ComponentMapping(
            vk::ComponentSwizzle::eIdentity,
          vk::ComponentSwizzle::eIdentity,
          vk::ComponentSwizzle::eIdentity
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
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while Device creation: " << e.what();
    return StatusCode::imageViewsCreationError;
  }
  return StatusCode::success;
}

static StatusCode readFile(const std::string& filename, std::vector<char>& buffer) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    return StatusCode::fileCouldntBeOpened;
  }

  size_t fileSize = (size_t) file.tellg();
  buffer.resize(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return StatusCode::success;
}

StatusCode VideoManager::createShaderModule(const std::vector<char>& code, vk::ShaderModule& shaderModule) {
  try {

    vk::ShaderModuleCreateInfo createInfo(
      {},
      code.size(),
      reinterpret_cast<const uint32_t*>(code.data())
    );

    shaderModule = device.createShaderModule(createInfo);

  } catch(vk::SystemError& e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while shader module creation: " << e.what();
    return StatusCode::shaderModuleCreationError;
  }

  return StatusCode::success;
}

StatusCode VideoManager::createRenderPass() {
  
  BOOST_LOG_TRIVIAL(info) << "Creating render pass.";

  vk::AttachmentDescription colorAttachment(
    {},
    swapChainImageFormat,
    vk::SampleCountFlagBits::e1,
    vk::AttachmentLoadOp::eClear,
    vk::AttachmentStoreOp::eStore,
    vk::AttachmentLoadOp::eDontCare,
    vk::AttachmentStoreOp::eDontCare,
    vk::ImageLayout::eUndefined,
    vk::ImageLayout::ePresentSrcKHR
  );

  vk::AttachmentReference colorAttachmentRef(
    0,
    vk::ImageLayout::eColorAttachmentOptimal
  );

  vk::SubpassDescription subpass(
    {},
    vk::PipelineBindPoint::eGraphics,
    0,
    nullptr,
    1,
    &colorAttachmentRef
  );

  vk::RenderPassCreateInfo createInfo(
    {},
    1,
    &colorAttachment,
    1,
    &subpass
  );

  try {
    renderPass = device.createRenderPass(createInfo);
  
  } catch (vk::SystemError& e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while render pass creation: " << e.what();
    return StatusCode::renderPassCreationError;
  }

  return StatusCode::success;
}

StatusCode VideoManager::createGraphicsPipeline() {

  BOOST_LOG_TRIVIAL(info) << "Creating graphics pipeline.";

  std::vector<char> vertShaderCode;
  if (readFile("shaders/first.vert.spv", vertShaderCode) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't open vertex shader.";
    return StatusCode::graphicsPipelineCreationError;
  }

  std::vector<char> fragShaderCode;
  if (readFile("shaders/first.frag.spv", fragShaderCode) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't open fragment shader.";
    return StatusCode::graphicsPipelineCreationError;
  }

  if (createShaderModule(vertShaderCode, vertShaderModule) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create vertex shader module.";
    return StatusCode::graphicsPipelineCreationError;
  }

  if (createShaderModule(fragShaderCode, fragShaderModule) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create fragment shader module.";
    return StatusCode::graphicsPipelineCreationError;
  }

  vk::PipelineShaderStageCreateInfo vertCreateInfo(
    {},
    vk::ShaderStageFlagBits::eVertex,
    vertShaderModule,
    "main"
  );

  vk::PipelineShaderStageCreateInfo fragCreateInfo(
    {},
    vk::ShaderStageFlagBits::eFragment,
    fragShaderModule,
    "main"
  );

  try {
    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertCreateInfo, fragCreateInfo};

    vk::PipelineVertexInputStateCreateInfo vertStateCreateInfo(
      {},
      0,
      nullptr,
      0,
      nullptr
    );

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
      {},
      vk::PrimitiveTopology::eTriangleList,
      vk::False
    );

    vk::Viewport viewport(
      0.0f,
      0.0f,
      static_cast<float>(swapChainExtent.width),
      static_cast<float>(swapChainExtent.height),
      0.0f,
      1.0f
    );

    vk::Rect2D scissor(
      {0,0},
      swapChainExtent
    );

    vk::PipelineDynamicStateCreateInfo dynamicState(
      {},
      static_cast<uint32_t>(dynamicStates.size()),
      dynamicStates.data()
    );

    vk::PipelineViewportStateCreateInfo viewportState(
      {},
      1,
      &viewport,
      1,
      &scissor
    );

    vk::PipelineRasterizationStateCreateInfo rasterizer(
      {},
      vk::False,
      vk::False,
      vk::PolygonMode::eFill,
      vk::CullModeFlagBits::eBack,      
      vk::FrontFace::eCounterClockwise,
      vk::False,
      0.0f,
      0.0f,
      0.0f,
      1.0f
    );

    vk::PipelineMultisampleStateCreateInfo multisampling(
      {},
      vk::SampleCountFlagBits::e1,
      vk::False,
      1.0f,
      nullptr,
      vk::False,
      vk::False);

    vk::PipelineColorBlendAttachmentState colorBlendAttachment(
      vk::False,
      vk::BlendFactor::eOne,
      vk::BlendFactor::eZero,
      vk::BlendOp::eAdd,
      vk::BlendFactor::eOne,
      vk::BlendFactor::eZero,
      vk::BlendOp::eAdd
    );

    vk::PipelineColorBlendStateCreateInfo colorBlending(
      {},
      vk::False,
      vk::LogicOp::eCopy,
      1,
      &colorBlendAttachment,
      {0.0f, 0.0f, 0.0f, 0.0f}
    );
    
    vk::PipelineLayoutCreateInfo pipelineCreateInfo(
      {},
      0,
      nullptr,
      0,
      nullptr
    );

    vk::GraphicsPipelineCreateInfo createInfo(
      {},
      2,
      shaderStages,
      &vertStateCreateInfo,
      &inputAssembly,
      nullptr,
      &viewportState,
      &rasterizer,
      &multisampling,
      nullptr,
      &colorBlending,
      &dynamicState,
      pipelineLayout,
      renderPass,
      0,
      nullptr,
      -1
    );

    auto [resultPipelineCreation, pipeline] = device.createGraphicsPipeline(nullptr, createInfo);
    graphicsPipeline = pipeline;

    if (resultPipelineCreation != vk::Result::eSuccess) {
      BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while graphics pipeline creation.";
      return StatusCode::shaderModuleCreationError;
    }

  } catch (vk::SystemError& e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while graphics pipeline creation: " << e.what();
    return StatusCode::shaderModuleCreationError;
  }
  return StatusCode::success;
}

StatusCode VideoManager::createFramebuffers() {
  BOOST_LOG_TRIVIAL(info) << "Creating framebuffers.";
  swapChainFramebuffers.resize(swapChainImageViews.size());
  try {
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
      vk::ImageView attachments[] = {
        swapChainImageViews[i]
      };

      vk::FramebufferCreateInfo framebufferInfo(
        {},
        renderPass,
        1,
        attachments,
        swapChainExtent.width,
        swapChainExtent.height,
        1

      );
      
      if (device.createFramebuffer(&framebufferInfo, nullptr, &swapChainFramebuffers[i]) != vk::Result::eSuccess) {
        BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while framebuffer number " + std::to_string(i) + ".";
        return StatusCode::shaderModuleCreationError;
      }
    }

  } catch (vk::SystemError& e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while framebuffers creation: " << e.what();
    return StatusCode::shaderModuleCreationError;
  }

  return StatusCode::success;
}

void VideoManager::dismantle() {

  BOOST_LOG_TRIVIAL(info) << "Destroying framebuffers.";
  for (auto framebuffer : swapChainFramebuffers) {
    device.destroyFramebuffer(framebuffer);
  }

  BOOST_LOG_TRIVIAL(info) << "Destroying pipeline.";
  device.destroyPipeline(graphicsPipeline);

  BOOST_LOG_TRIVIAL(info) << "Destroying pipeline layout.";
  device.destroyPipelineLayout(pipelineLayout);

  BOOST_LOG_TRIVIAL(info) << "Destroying render pass.";
  device.destroyRenderPass(renderPass);

  BOOST_LOG_TRIVIAL(info) << "Destroying pipeline layout.";
  device.destroyPipelineLayout(pipelineLayout);

  BOOST_LOG_TRIVIAL(info) << "Destroying image views.";
  for (auto imageView : swapChainImageViews) {
    device.destroyImageView(imageView);
  }

  BOOST_LOG_TRIVIAL(info) << "Destroying swapchain.";
  device.destroySwapchainKHR(swapchain);
  
  BOOST_LOG_TRIVIAL(info) << "Destroying surface.";
  instance.destroySurfaceKHR(surface);

  BOOST_LOG_TRIVIAL(info) << "Destroying shader modules.";
  device.destroyShaderModule(vertShaderModule);
  device.destroyShaderModule(fragShaderModule);

  BOOST_LOG_TRIVIAL(info) << "Destroying device.";
  device.destroy();

  BOOST_LOG_TRIVIAL(info) << "Destroying instance.";
  instance.destroy();
}

VideoManager mVideoManager;

} // namespace benpu