/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "vulkanrenderer.h"
#include <qalgorithms.h>
#include <QVector>
#include <QDebug>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallbackFunc(VkDebugReportFlagsEXT flags,
                                                        VkDebugReportObjectTypeEXT objectType,
                                                        uint64_t object,
                                                        size_t location,
                                                        int32_t messageCode,
                                                        const char *pLayerPrefix,
                                                        const char *pMessage,
                                                        void *pUserData)
{
    Q_UNUSED(flags);
    Q_UNUSED(objectType);
    Q_UNUSED(object);
    Q_UNUSED(location);
    Q_UNUSED(pUserData);
    qDebug("DEBUG: %s: %d: %s", pLayerPrefix, messageCode, pMessage);
    return VK_FALSE;
}

void VulkanRenderer::createDeviceAndSurface()
{
    VkApplicationInfo appInfo;
    memset(&appInfo, 0, sizeof(appInfo));
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "qqvk";
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 2);

    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    qDebug("%d instance layers", layerCount);
    QVector<char *> enabledLayers;
    if (layerCount) {
        QVector<VkLayerProperties> layerProps(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layerProps.data());
        for (const VkLayerProperties &p : qAsConst(layerProps)) {
            if (m_flags.testFlag(EnableValidation) && !strcmp(p.layerName, "VK_LAYER_LUNARG_standard_validation"))
                enabledLayers.append(strdup(p.layerName));
        }
    }
    if (!enabledLayers.isEmpty())
        qDebug() << "enabling instance layers" << enabledLayers;

    m_hasDebug = false;
    uint32_t extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    qDebug("%d instance extensions", extCount);
    QVector<char *> enabledExtensions;
    if (extCount) {
        QVector<VkExtensionProperties> extProps(extCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extCount, extProps.data());
        for (const VkExtensionProperties &p : qAsConst(extProps)) {
            if (!strcmp(p.extensionName, "VK_EXT_debug_report")) {
                enabledExtensions.append(strdup(p.extensionName));
                m_hasDebug = true;
            } else if (!strcmp(p.extensionName, "VK_KHR_surface")
                       || !strcmp(p.extensionName, "VK_KHR_win32_surface"))
            {
                enabledExtensions.append(strdup(p.extensionName));
            }
        }
    }
    if (!enabledExtensions.isEmpty())
        qDebug() << "enabling instance extensions" << enabledExtensions;

    VkInstanceCreateInfo instInfo;
    memset(&instInfo, 0, sizeof(instInfo));
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pApplicationInfo = &appInfo;
    if (!enabledLayers.isEmpty()) {
        instInfo.enabledLayerCount = enabledLayers.count();
        instInfo.ppEnabledLayerNames = enabledLayers.constData();
    }
    if (!enabledExtensions.isEmpty()) {
        instInfo.enabledExtensionCount = enabledExtensions.count();
        instInfo.ppEnabledExtensionNames = enabledExtensions.constData();
    }

    VkResult err = vkCreateInstance(&instInfo, nullptr, &m_vkInst);
    if (err != VK_SUCCESS)
        qFatal("Failed to create Vulkan instance: %d", err);

    for (auto s : enabledLayers) free(s);
    for (auto s : enabledExtensions) free(s);

    if (m_hasDebug) {
        vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(m_vkInst, "vkCreateDebugReportCallbackEXT"));
        vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(m_vkInst, "vkDestroyDebugReportCallbackEXT"));
        vkDebugReportMessageEXT = reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(m_vkInst, "vkDebugReportMessageEXT"));

        VkDebugReportCallbackCreateInfoEXT dbgCallbackInfo;
        memset(&dbgCallbackInfo, 0, sizeof(dbgCallbackInfo));
        dbgCallbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
        dbgCallbackInfo.flags =  VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        dbgCallbackInfo.pfnCallback = &debugCallbackFunc;
        err = vkCreateDebugReportCallbackEXT(m_vkInst, &dbgCallbackInfo, nullptr, &m_debugCallback);
        if (err != VK_SUCCESS) {
            qWarning("Failed to create debug report callback: %d", err);
            m_hasDebug = false;
        }
    }

    createSurface();

    uint32_t devCount = 0;
    vkEnumeratePhysicalDevices(m_vkInst, &devCount, nullptr);
    qDebug("%d physical devices", devCount);
    if (!devCount)
        qFatal("No physical devices");
    // Just pick the first physical device for now.
    devCount = 1;
    err = vkEnumeratePhysicalDevices(m_vkInst, &devCount, &m_vkPhysDev);
    if (err != VK_SUCCESS)
        qFatal("Failed to enumerate physical devices: %d", err);

    VkPhysicalDeviceProperties physDevProps;
    vkGetPhysicalDeviceProperties(m_vkPhysDev, &physDevProps);
    qDebug("Device name: %s\nDriver version: %d.%d.%d", physDevProps.deviceName,
           VK_VERSION_MAJOR(physDevProps.driverVersion), VK_VERSION_MINOR(physDevProps.driverVersion),
           VK_VERSION_PATCH(physDevProps.driverVersion));

    layerCount = 0;
    vkEnumerateDeviceLayerProperties(m_vkPhysDev, &layerCount, nullptr);
    qDebug("%d device layers", layerCount);
    enabledLayers.clear();
    if (layerCount) {
        QVector<VkLayerProperties> layerProps(layerCount);
        vkEnumerateDeviceLayerProperties(m_vkPhysDev, &layerCount, layerProps.data());
        for (const VkLayerProperties &p : qAsConst(layerProps)) {
            // If the validation layer is enabled for the instance, it has to
            // be enabled for the device too, otherwise be prepared for
            // mysterious errors...
            if (m_flags.testFlag(EnableValidation) && !strcmp(p.layerName, "VK_LAYER_LUNARG_standard_validation"))
                enabledLayers.append(strdup(p.layerName));
        }
    }
    if (!enabledLayers.isEmpty())
        qDebug() << "enabling device layers" << enabledLayers;

    extCount = 0;
    vkEnumerateDeviceExtensionProperties(m_vkPhysDev, nullptr, &extCount, nullptr);
    qDebug("%d device extensions", extCount);
    enabledExtensions.clear();
    if (extCount) {
        QVector<VkExtensionProperties> extProps(extCount);
        vkEnumerateDeviceExtensionProperties(m_vkPhysDev, nullptr, &extCount, extProps.data());
        for (const VkExtensionProperties &p : qAsConst(extProps)) {
            if (!strcmp(p.extensionName, "VK_KHR_swapchain")
                || !strcmp(p.extensionName, "VK_NV_glsl_shader"))
            {
                enabledExtensions.append(strdup(p.extensionName));
            }
        }
    }
    if (!enabledExtensions.isEmpty())
        qDebug() << "enabling device extensions" << enabledExtensions;

    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysDev, &queueCount, nullptr);
    QVector<VkQueueFamilyProperties> queueFamilyProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysDev, &queueCount, queueFamilyProps.data());
    uint32_t gfxQueueFamilyIdx = -1;
    for (int i = 0; i < queueFamilyProps.count(); ++i) {
        qDebug("queue family %d: flags=0x%x count=%d", i, queueFamilyProps[i].queueFlags, queueFamilyProps[i].queueCount);
        bool ok = (queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
        ok |= physicalDeviceSupportsPresent(i);
        if (ok) {
            gfxQueueFamilyIdx = i;
            break;
        }
    }
    if (gfxQueueFamilyIdx == -1)
        qFatal("No presentable graphics queue family found");

    VkDeviceQueueCreateInfo queueInfo;
    memset(&queueInfo, 0, sizeof(queueInfo));
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = gfxQueueFamilyIdx;
    queueInfo.queueCount = 1;
    const float prio[] = { 0 };
    queueInfo.pQueuePriorities = prio;

    VkDeviceCreateInfo devInfo;
    memset(&devInfo, 0, sizeof(devInfo));
    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.queueCreateInfoCount = 1;
    devInfo.pQueueCreateInfos = &queueInfo;
    if (!enabledLayers.isEmpty()) {
        devInfo.enabledLayerCount = enabledLayers.count();
        devInfo.ppEnabledLayerNames = enabledLayers.constData();
    }
    if (!enabledExtensions.isEmpty()) {
        devInfo.enabledExtensionCount = enabledExtensions.count();
        devInfo.ppEnabledExtensionNames = enabledExtensions.constData();
    }

    err = vkCreateDevice(m_vkPhysDev, &devInfo, nullptr, &m_vkDev);
    if (err != VK_SUCCESS)
        qFatal("Failed to create device: %d", err);

    for (auto s : enabledLayers) free(s);
    for (auto s : enabledExtensions) free(s);

    vkGetDeviceQueue(m_vkDev, gfxQueueFamilyIdx, 0, &m_vkQueue);

    VkCommandPoolCreateInfo poolInfo;
    memset(&poolInfo, 0, sizeof(poolInfo));
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = gfxQueueFamilyIdx;
    err = vkCreateCommandPool(m_vkDev, &poolInfo, nullptr, &m_vkCmdPool);
    if (err != VK_SUCCESS)
        qFatal("Failed to create command pool: %d", err);

    m_hostVisibleMemIndex = 0;
    vkGetPhysicalDeviceMemoryProperties(m_vkPhysDev, &m_vkPhysDevMemProps);
    for (uint32_t i = 0; i < m_vkPhysDevMemProps.memoryTypeCount; ++i) {
        const VkMemoryType *memType = m_vkPhysDevMemProps.memoryTypes;
        qDebug("phys dev mem prop %d: flags=0x%x", i, memType[i].propertyFlags);
        if (memType[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            m_hostVisibleMemIndex = i;
    }

    m_colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
}

void VulkanRenderer::releaseDeviceAndSurface()
{
    releaseSurface();
    vkDestroyCommandPool(m_vkDev, m_vkCmdPool, nullptr);
    vkDestroyDevice(m_vkDev, nullptr);

    if (m_hasDebug)
        vkDestroyDebugReportCallbackEXT(m_vkInst, m_debugCallback, nullptr);

    vkDestroyInstance(m_vkInst, nullptr);
}

void VulkanRenderer::transitionImage(VkCommandBuffer cmdBuf,
                                     VkImage image,
                                     VkImageLayout oldLayout, VkImageLayout newLayout,
                                     VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                     bool ds)
{
    VkImageMemoryBarrier barrier;
    memset(&barrier, 0, sizeof(barrier));
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = !ds ? VK_IMAGE_ASPECT_COLOR_BIT : (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    barrier.subresourceRange.levelCount = barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);
}
