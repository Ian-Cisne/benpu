#ifndef STATUS_CODE_H
#define STATUS_CODE_H

enum StatusCode: int {
    success = 0,
    vulkanLibraryLoadError,
    vulkanExportedFunctionLoadError,
    vlukanGlobalFunctionLoadError
};

#endif
