
#include <boost/log/trivial.hpp>

#include "render/vulkan/command_pool.h"

namespace benpu {

CommandPool::CommandPool(vk::Device& device): device{device} {

}

StatusCode CommandPool::initialize(const QueueFamilyIndices& queueFamilyIndices) {

  BOOST_LOG_TRIVIAL(info) << "Creating command pool.";

  vk::CommandPoolCreateInfo poolInfo(
    {vk::CommandPoolCreateFlagBits::eResetCommandBuffer},
    queueFamilyIndices.graphicsFamily.value()
  );

  try {

    if (device.createCommandPool(&poolInfo, nullptr, &commandPool) != vk::Result::eSuccess) {
      BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while command pool creation.";
      return StatusCode::commandPoolCreationError;
    }

  } catch (vk::SystemError& e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while command pool creation: " << e.what();
    return StatusCode::commandPoolCreationError;
  }

  return StatusCode::success;
}

StatusCode CommandPool::createCommandBuffer() {

  BOOST_LOG_TRIVIAL(info) << "Creating command buffer.";

  vk::CommandBufferAllocateInfo allocInfo(
    commandPool,
    vk::CommandBufferLevel::ePrimary,
    1
  );

  try {
    
    if (device.allocateCommandBuffers(&allocInfo, &commandBuffer) != vk::Result::eSuccess) {
      BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while command buffer creation.";
      return StatusCode::commandBufferCreationError;
    }

  } catch (vk::SystemError& e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while command buffer creation: " << e.what();
    return StatusCode::commandBufferCreationError;
  }

  return StatusCode::success;
}

StatusCode CommandPool::createSyncObjects() {

  BOOST_LOG_TRIVIAL(info) << "Creating synchronization objects.";

  vk::SemaphoreCreateInfo sCreateInfo;
  
  if (device.createSemaphore(&sCreateInfo, nullptr, &imageAvailableSemaphore) != vk::Result::eSuccess) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while image semaphore creation.";
    return StatusCode::semaphoreCreationError;
  }
  
  if (device.createSemaphore(&sCreateInfo, nullptr, &renderFinishedSemaphore) != vk::Result::eSuccess) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while render semaphore creation.";
    return StatusCode::semaphoreCreationError;
  }

  vk::FenceCreateInfo fCreateInfo(
    (
      vk::FenceCreateFlagBits::eSignaled
    )
  );
  
  if (device.createFence(&fCreateInfo, nullptr, &inFlightFence) != vk::Result::eSuccess) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while fence creation.";
    return StatusCode::fenceCreationError;
  }

  return StatusCode::success;
}

} //namespace benpu

