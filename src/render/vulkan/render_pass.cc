
#include <boost/log/trivial.hpp>

#include "render/vulkan/render_pass.h"

namespace benpu {

RenderPass::RenderPass(vk::Device& device): device{device} {}

StatusCode RenderPass::initialize(vk::Format swapChainImageFormat) {
  
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
  
  vk::SubpassDependency dependency(
    vk::SubpassExternal,
    0,
    (vk::PipelineStageFlagBits::eColorAttachmentOutput),
    (vk::PipelineStageFlagBits::eColorAttachmentOutput),
    vk::AccessFlagBits::eNone,
    vk::AccessFlagBits::eNone
  );

  vk::RenderPassCreateInfo createInfo(
    {},
    1,
    &colorAttachment,
    1,
    &subpass,
    1,
    &dependency
  );
  
  try {

    renderPass = device.createRenderPass(createInfo);
  
  } catch (vk::SystemError& e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while render pass creation: " << e.what();
    return StatusCode::renderPassCreationError;
  }

  return StatusCode::success;
}

vk::RenderPass RenderPass::getRenderPass() const {
  return renderPass;
}

} // namespace benpu