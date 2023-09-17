#ifndef STATUS_CODE_H
#define STATUS_CODE_H

enum StatusCode: int {
    success = 0,
    vulkanError,
    extensionNotFound,
    layerNotFound,
    noPhysicalDeviceFound,
    noProprerPhysicalDeviceFound,
    imageViewsCreationError,
    deviceCreationError,
    swapchainreationError,
    fileCouldntBeOpened,
    graphicsPipelineCreationError,
    shaderModuleCreationError,
    renderPassCreationError,
    frameBufferCreationError,
    commandPoolCreationError
};

#endif
