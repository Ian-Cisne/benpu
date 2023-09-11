#include "video_manager.h"

#include <algorithm>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <memory>
#include <set>
#include <tuple>
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

  QueueFamilyIndices queueFamilyIndices;
  if(pickPhysicalDevice(queueFamilyIndices) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't pick a physical device.";
  }

  if(createDevice(queueFamilyIndices) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't pick a physical device.";
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
int VideoManager::getBestPhysicalDevice(const std::vector<vk::PhysicalDevice>& physicalDevices, VideoManager::QueueFamilyIndices& bestDeviceQueueFamilyIndices) {

  int bestPhysicalDevicesIndex = -1;
  int bestScore = 0;

  for(int i = 0; i - physicalDevices.size(); ++i) {
    QueueFamilyIndices queueFamilyIndices;
    vk::PhysicalDeviceProperties deviceProperties = physicalDevices[i].getProperties();
    vk::PhysicalDeviceFeatures deviceFeatures = physicalDevices[i].getFeatures();
    
    std::vector<vk::QueueFamilyProperties> queueFamiliesProperties = physicalDevices[i].getQueueFamilyProperties();

    int score = 0;
    
    // Consider the device type. Discrete GPUs get a higher score.
    if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        score += 1000; // Discrete GPU gets a high score
    } else if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
        score += 500; // Integrated GPU gets a moderate score
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
      || !queueFamilyIndices.isComplete()) {
        //If it does support the queue families we need, we can't use it.
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

StatusCode VideoManager::pickPhysicalDevice(QueueFamilyIndices &queueFamilyIndices) {
  auto availablePhysicalDevices = instance.enumeratePhysicalDevices();

  if (availablePhysicalDevices.size() == 0) {
    return StatusCode::noPhysicalDeviceFound;
  }
  int bestPhysicalDeviceIndex;
  bestPhysicalDeviceIndex = getBestPhysicalDevice(availablePhysicalDevices, queueFamilyIndices);
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

StatusCode VideoManager::createDevice(QueueFamilyIndices& queueFamilyIndices) {
  try {
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
      {},
      {},
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

void VideoManager::dismantle() {
  
  BOOST_LOG_TRIVIAL(info) << "Destroying surface.";
  
  vkDestroySurfaceKHR(instance, surface, nullptr);

  BOOST_LOG_TRIVIAL(info) << "Destroying device.";
  device.destroy();

  BOOST_LOG_TRIVIAL(info) << "Destroying instance.";
  instance.destroy();
}

VideoManager mVideoManager;

} // namespace benpu
