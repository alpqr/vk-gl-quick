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
#include <QRect>
#include <QVector>
#include <QDebug>

static inline VkDeviceSize alignedSize(VkDeviceSize size, VkDeviceSize byteAlign)
{
    return (size + byteAlign - 1) & ~(byteAlign - 1);
}

static inline VkRect2D rect2D(const QRect &rect)
{
    VkRect2D r;
    r.offset.x = rect.x();
    r.offset.y = rect.y();
    r.extent.width = rect.width();
    r.extent.height = rect.height();
    return r;
}

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

void VulkanRenderer::createDevice()
{
    VkApplicationInfo appInfo;
    memset(&appInfo, 0, sizeof(appInfo));
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "qqvk";
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 2);

    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    qDebug("%d layers", layerCount);
    QVector<char *> enabledLayers;
    if (layerCount) {
        QVector<VkLayerProperties> layerProps(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layerProps.data());
        for (const VkLayerProperties &p : qAsConst(layerProps)) {
            if (!strcmp(p.layerName, "VK_LAYER_LUNARG_standard_validation"))
                enabledLayers.append(strdup(p.layerName));
        }
    }
    if (!enabledLayers.isEmpty())
        qDebug() << "enabling layers" << enabledLayers;

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
    uint32_t gfxQueueFamilyIdx = 0;
    for (int i = 0; i < queueFamilyProps.count(); ++i) {
        qDebug("queue family %d: flags=0x%x count=%d", i, queueFamilyProps[i].queueFlags, queueFamilyProps[i].queueCount);
        if (queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            gfxQueueFamilyIdx = i;
    }

    VkDeviceQueueCreateInfo queueInfo;
    memset(&queueInfo, 0, sizeof(queueInfo));
    queueInfo.queueFamilyIndex = gfxQueueFamilyIdx;
    queueInfo.queueCount = 1;

    VkDeviceCreateInfo devInfo;
    memset(&devInfo, 0, sizeof(devInfo));
    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.queueCreateInfoCount = 1;
    devInfo.pQueueCreateInfos = &queueInfo;
    devInfo.enabledExtensionCount = enabledExtensions.count();
    devInfo.ppEnabledExtensionNames = enabledExtensions.constData();

    err = vkCreateDevice(m_vkPhysDev, &devInfo, nullptr, &m_vkDev);
    if (err != VK_SUCCESS)
        qFatal("Failed to create device: %d", err);

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
}

void VulkanRenderer::releaseDevice()
{
    vkDestroyCommandPool(m_vkDev, m_vkCmdPool, nullptr);

    vkDestroyDevice(m_vkDev, nullptr);

    if (m_hasDebug)
        vkDestroyDebugReportCallbackEXT(m_vkInst, m_debugCallback, nullptr);

    vkDestroyInstance(m_vkInst, nullptr);
}

void VulkanRenderer::createRenderTarget(const QSize &size)
{
    VkImageCreateInfo imgInfo;
    memset(&imgInfo, 0, sizeof(imgInfo));
    imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.imageType = VK_IMAGE_TYPE_2D;
    imgInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imgInfo.extent.width = size.width();
    imgInfo.extent.height = size.height();
    imgInfo.mipLevels = 1;
    imgInfo.arrayLayers = 1;
    imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imgInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkResult err = vkCreateImage(m_vkDev, &imgInfo, nullptr, &m_color);
    if (err != VK_SUCCESS)
        qFatal("Failed to create color attachment: %d", err);

    imgInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
    imgInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    err = vkCreateImage(m_vkDev, &imgInfo, nullptr, &m_ds);
    if (err != VK_SUCCESS)
        qFatal("Failed to create depth-stencil attachment: %d", err);

    VkMemoryRequirements colorMemReq;
    vkGetImageMemoryRequirements(m_vkDev, m_color, &colorMemReq);
    uint memTypeIndex = 0;
    if (colorMemReq.memoryTypeBits)
        memTypeIndex = qCountTrailingZeroBits(colorMemReq.memoryTypeBits);

    VkMemoryRequirements dsMemReq;
    vkGetImageMemoryRequirements(m_vkDev, m_ds, &dsMemReq);

    VkMemoryAllocateInfo memInfo;
    memset(&memInfo, 0, sizeof(memInfo));
    memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    const VkDeviceSize dsOffset = alignedSize(colorMemReq.size, dsMemReq.alignment);
    memInfo.allocationSize = dsOffset + dsMemReq.size;
    memInfo.memoryTypeIndex = memTypeIndex;
    qDebug("allocating %lu bytes for color and depth-stencil", memInfo.allocationSize);

    err = vkAllocateMemory(m_vkDev, &memInfo, nullptr, &m_rtMem);
    if (err != VK_SUCCESS)
        qFatal("Failed to allocate image memory: %d", err);

    err = vkBindImageMemory(m_vkDev, m_color, m_rtMem, 0);
    if (err != VK_SUCCESS)
        qFatal("Failed to bind image memory for color: %d", err);

    err = vkBindImageMemory(m_vkDev, m_ds, m_rtMem, dsOffset);
    if (err != VK_SUCCESS)
        qFatal("Failed to bind image memory for depth-stencil: %d", err);

    VkImageViewCreateInfo imgViewInfo;
    memset(&imgViewInfo, 0, sizeof(imgViewInfo));
    imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imgViewInfo.image = m_color;
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imgViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    imgViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    imgViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    imgViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imgViewInfo.subresourceRange.levelCount = imgViewInfo.subresourceRange.layerCount = 1;
    err = vkCreateImageView(m_vkDev, &imgViewInfo, nullptr, &m_colorView);
    if (err != VK_SUCCESS)
        qFatal("Failed to create color attachment view: %d", err);

    imgViewInfo.image = m_ds;
    imgViewInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
    err = vkCreateImageView(m_vkDev, &imgViewInfo, nullptr, &m_dsView);
    if (err != VK_SUCCESS)
        qFatal("Failed to create depth-stencil attachment view: %d", err);

    VkAttachmentDescription attDesc[2];
    memset(attDesc, 0, sizeof(attDesc));
    attDesc[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attDesc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attDesc[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attDesc[1].format = VK_FORMAT_D24_UNORM_S8_UINT;
    attDesc[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attDesc[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attDesc[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // do not write out depth
    attDesc[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attDesc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference dsRef = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    VkSubpassDescription subPassDesc;
    memset(&subPassDesc, 0, sizeof(subPassDesc));
    subPassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPassDesc.colorAttachmentCount = 1;
    subPassDesc.pColorAttachments = &colorRef;
    subPassDesc.pDepthStencilAttachment = &dsRef;

    VkRenderPassCreateInfo rpInfo;
    memset(&rpInfo, 0, sizeof(rpInfo));
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.attachmentCount = 2;
    rpInfo.pAttachments = attDesc;
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &subPassDesc;
    err = vkCreateRenderPass(m_vkDev, &rpInfo, nullptr, &m_renderPass);
    if (err != VK_SUCCESS)
        qFatal("Failed to create renderpass: %d", err);

    VkImageView views[2] = { m_colorView, m_dsView };

    VkFramebufferCreateInfo fbInfo;
    memset(&fbInfo, 0, sizeof(fbInfo));
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.renderPass = m_renderPass;
    fbInfo.attachmentCount = 2;
    fbInfo.pAttachments = views;
    fbInfo.width = size.width();
    fbInfo.height = size.height();
    fbInfo.layers = 1;
    err = vkCreateFramebuffer(m_vkDev, &fbInfo, nullptr, &m_fb);
    if (err != VK_SUCCESS)
        qFatal("Failed to create framebuffer: %d", err);
}

void VulkanRenderer::releaseRenderTarget()
{
    if (m_renderPass) {
        vkDestroyRenderPass(m_vkDev, m_renderPass, nullptr);
        m_renderPass = 0;
    }
    if (m_fb) {
        vkDestroyFramebuffer(m_vkDev, m_fb, nullptr);
        m_fb = 0;
    }
    if (m_dsView) {
        vkDestroyImageView(m_vkDev, m_dsView, nullptr);
        m_dsView = 0;
    }
    if (m_ds) {
        vkDestroyImage(m_vkDev, m_ds, nullptr);
        m_ds = 0;
    }
    if (m_colorView) {
        vkDestroyImageView(m_vkDev, m_colorView, nullptr);
        m_colorView = 0;
    }
    if (m_color) {
        vkDestroyImage(m_vkDev, m_color, nullptr);
        m_color = 0;
    }
    if (m_rtMem) {
        vkFreeMemory(m_vkDev, m_rtMem, nullptr);
        m_rtMem = 0;
    }
}

void VulkanRenderer::render(const QSize &size)
{
    vkResetCommandPool(m_vkDev, m_vkCmdPool, 0);
    VkCommandBuffer cmdBuf;
    VkCommandBufferAllocateInfo cmdBufInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, m_vkCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1 };
    VkResult err = vkAllocateCommandBuffers(m_vkDev, &cmdBufInfo, &cmdBuf);
    if (err != VK_SUCCESS)
        qFatal("Failed to allocate command buffer: %d", err);

    VkCommandBufferBeginInfo cmdBufBeginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, 0, nullptr };
    vkBeginCommandBuffer(cmdBuf, &cmdBufBeginInfo);

    VkRenderPassBeginInfo rpBeginInfo;
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBeginInfo.pNext = nullptr;
    rpBeginInfo.renderPass = m_renderPass;
    rpBeginInfo.framebuffer = m_fb;
    rpBeginInfo.renderArea = rect2D(QRect(QPoint(0, 0), size));
    rpBeginInfo.clearValueCount = 1;
    VkClearColorValue clearColor = { 0.0f, 1.0f, 0.0f, 1.0f };
    VkClearValue clearValue;
    clearValue.color = clearColor;
    clearValue.depthStencil.depth = 1.0f;
    clearValue.depthStencil.stencil = 0;
    rpBeginInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = { 0, 0, float(size.width()), float(size.height()), 0, 1 };
    vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

    VkImageSubresourceRange subResRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    vkCmdClearColorImage(cmdBuf, m_color, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &clearColor, 1, &subResRange);
    // ### do something else instead of the clear

    vkCmdEndRenderPass(cmdBuf);
    vkEndCommandBuffer(cmdBuf);

    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(submitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuf;
    err = vkQueueSubmit(m_vkQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (err != VK_SUCCESS) {
        qWarning("Failed to submit to command queue: %d", err);
        return;
    }
}
