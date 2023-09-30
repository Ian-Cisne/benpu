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
    swapchainCreationError,
    fileCouldntBeOpened,
    graphicsPipelineCreationError,
    shaderModuleCreationError,
    renderPassCreationError,
    frameBufferCreationError,
    commandPoolCreationError,
    commandBufferCreationError,
    commandBufferRecordError,
    semaphoreCreationError,
    fenceCreationError,
    extensionNotPresent,
    queueCreationError
};

enum ObjectStatus {
    unitialized,
    ok,
    error
};
#endif
