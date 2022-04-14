// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <string.h>
#include <stdbool.h>

#include "XLinkPublicDefines.h"
#include "XLinkPlatform.h"
#include "XLinkPlatformErrorUtils.h"
#include "usb_host.h"
#include "pcie_host.h"
#include "tcpip_host.h"
#include "XLinkStringUtils.h"
#include "PlatformDeviceFd.h"

#define MVLOG_UNIT_NAME PlatformDeviceControl
#include "XLinkLog.h"

// ------------------------------------
// XLinkPlatform API implementation. Begin.
// ------------------------------------

void XLinkPlatformInit(void* options)
{
    usbInitialize(options);
    tcpipPlatformInitialize();
}


int XLinkPlatformBootRemote(const deviceDesc_t* deviceDesc, const char* binaryPath)
{
    FILE *file;
    long file_size;

    char *image_buffer;

    /* Open the mvcmd file */
    file = fopen(binaryPath, "rb");

    if(file == NULL) {
        mvLog(MVLOG_ERROR, "Cannot open file by path: %s", binaryPath);
        return -7;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);
    if(file_size <= 0 || !(image_buffer = (char*)malloc(file_size)))
    {
        mvLog(MVLOG_ERROR, "cannot allocate image_buffer. file_size = %ld", file_size);
        fclose(file);
        return -3;
    }
    if((long) fread(image_buffer, 1, file_size, file) != file_size)
    {
        mvLog(MVLOG_ERROR, "cannot read file to image_buffer");
        fclose(file);
        free(image_buffer);
        return -7;
    }
    fclose(file);

    if(XLinkPlatformBootFirmware(deviceDesc, image_buffer, file_size)) {
        free(image_buffer);
        return -1;
    }

    free(image_buffer);
    return 0;
}

int XLinkPlatformBootFirmware(const deviceDesc_t* deviceDesc, const char* firmware, size_t length) {

    switch (deviceDesc->protocol) {
        case X_LINK_USB_VSC:
        case X_LINK_USB_CDC:
            return usbPlatformBootFirmware(deviceDesc, firmware, length);

        case X_LINK_PCIE:
            return pciePlatformBootFirmware(deviceDesc, firmware, length);

        case X_LINK_TCP_IP:
            return tcpipPlatformBootFirmware(deviceDesc, firmware, length);

        default:
            return X_LINK_PLATFORM_INVALID_PARAMETERS;
    }

}


int XLinkPlatformConnect(const char* devPathRead, const char* devPathWrite, XLinkProtocol_t protocol, void** fd)
{
    switch (protocol) {
        case X_LINK_USB_VSC:
        case X_LINK_USB_CDC:
            return usbPlatformConnect(devPathRead, devPathWrite, fd);

        case X_LINK_PCIE:
            return pciePlatformConnect(devPathRead, devPathWrite, fd);

        case X_LINK_TCP_IP:
            return tcpipPlatformConnect(devPathRead, devPathWrite, fd);

        default:
            return X_LINK_PLATFORM_INVALID_PARAMETERS;
    }
}

int XLinkPlatformBootBootloader(const char* name, XLinkProtocol_t protocol)
{
    switch (protocol) {
        case X_LINK_USB_VSC:
        case X_LINK_USB_CDC:
            return usbPlatformBootBootloader(name);

        case X_LINK_PCIE:
            return pciePlatformBootBootloader(name);

        case X_LINK_TCP_IP:
            return tcpipPlatformBootBootloader(name);

        default:
            return X_LINK_PLATFORM_INVALID_PARAMETERS;
    }
}

int XLinkPlatformCloseRemote(xLinkDeviceHandle_t* deviceHandle)
{
    if(deviceHandle->protocol == X_LINK_ANY_PROTOCOL ||
       deviceHandle->protocol == X_LINK_NMB_OF_PROTOCOLS) {
        return X_LINK_PLATFORM_ERROR;
    }

    switch (deviceHandle->protocol) {
        case X_LINK_USB_VSC:
        case X_LINK_USB_CDC:
            return usbPlatformClose(deviceHandle->xLinkFD);

        case X_LINK_PCIE:
            return pciePlatformClose(deviceHandle->xLinkFD);

        case X_LINK_TCP_IP:
            return tcpipPlatformClose(deviceHandle->xLinkFD);

        default:
            return X_LINK_PLATFORM_INVALID_PARAMETERS;
    }

}

// ------------------------------------
// XLinkPlatform API implementation. End.
// ------------------------------------
