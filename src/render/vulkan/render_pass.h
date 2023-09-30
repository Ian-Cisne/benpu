#ifndef BENPU_RENDER_PASS_H_
#define BENPU_RENDER_PASS_H_

#include <vulkan/vulkan.hpp>

#include "status_code.h"

namespace benpu {

class RenderPass {
public:
  RenderPass(vk::Device& device);
  StatusCode initialize(vk::Format swapChainImageFormat);
  vk::RenderPass getRenderPass() const;

private:
  vk::Device& device;
  vk::RenderPass renderPass = nullptr;
};

} // namespace benpu

#endif