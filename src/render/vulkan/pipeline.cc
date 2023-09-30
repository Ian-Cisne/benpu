
#include <fstream>

#include <boost/log/trivial.hpp>

#include "render/vulkan/pipeline.h"

namespace benpu {

Pipeline::Pipeline(vk::Device& device): device{device} {

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

StatusCode Pipeline::createShaderModule(const std::vector<char>& code, vk::ShaderModule& shaderModule) {
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

StatusCode Pipeline::initialize() {

  BOOST_LOG_TRIVIAL(info) << "Creating graphics pipeline.";

  std::vector<char> vertexShaderCode;
  if (readFile("shaders/first.vert.spv", vertexShaderCode) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't open vertex shader.";
    return StatusCode::graphicsPipelineCreationError;
  }

  std::vector<char> fragmentShaderCode;
  if (readFile("shaders/first.frag.spv", fragmentShaderCode) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't open fragment shader.";
    return StatusCode::graphicsPipelineCreationError;
  }

  if (createShaderModule(vertexShaderCode, vertexShaderModule) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create vertex shader module.";
    return StatusCode::graphicsPipelineCreationError;
  }

  if (createShaderModule(fragmentShaderCode, fragmentShaderModule) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create fragment shader module.";
    return StatusCode::graphicsPipelineCreationError;
  }

  try {
    vk::PipelineShaderStageCreateInfo shaderStages[] = {
      vk::PipelineShaderStageCreateInfo(
        {},
        vk::ShaderStageFlagBits::eVertex,
        vertexShaderModule,
        "main"
      ), 
      vk::PipelineShaderStageCreateInfo(
        {},
        vk::ShaderStageFlagBits::eFragment,
        fragmentShaderModule,
        "main"
      )
    };

    vk::PipelineVertexInputStateCreateInfo vertInputInfo(
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

    vk::PipelineViewportStateCreateInfo viewportState(
      {},
      1,
      nullptr,
      1,
      nullptr
    );

    vk::PipelineRasterizationStateCreateInfo rasterizer(
      {},
      vk::False,
      vk::False,
      vk::PolygonMode::eFill,
      vk::CullModeFlagBits::eBack,      
      vk::FrontFace::eClockwise,
      vk::False,
      {},
      {},
      {},
      1.0f
    );

    vk::PipelineMultisampleStateCreateInfo multisampling(
      {},
      vk::SampleCountFlagBits::e1,
      vk::False
    );

    vk::PipelineColorBlendAttachmentState colorBlendAttachment(
      vk::True,
      vk::BlendFactor::eSrcAlpha,
      vk::BlendFactor::eOneMinusSrcAlpha,
      vk::BlendOp::eAdd,
      vk::BlendFactor::eOne,
      vk::BlendFactor::eZero,
      vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR |vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );

    vk::PipelineColorBlendStateCreateInfo colorBlending(
      {},
      vk::False,
      vk::LogicOp::eCopy,
      1,
      &colorBlendAttachment,
      {0.0f, 0.0f, 0.0f, 0.0f}
    );

    std::vector<vk::DynamicState> dynamicStates = {
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor,
      vk::DynamicState::eLineWidth
    };

    vk::PipelineDynamicStateCreateInfo dynamicState(
      {},
      static_cast<uint32_t>(dynamicStates.size()),
      dynamicStates.data()
    );
    
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
      {},
      0,
      nullptr,
      0,
      nullptr
    );


    if (device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess) {
      BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while pipeline layout creation.";
      return StatusCode::shaderModuleCreationError;
    }

    vk::GraphicsPipelineCreateInfo createInfo(
      {},
      2,
      shaderStages,
      &vertInputInfo,
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
      nullptr
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

} //namespace benpu