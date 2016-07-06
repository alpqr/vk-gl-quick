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

#include "vulkanwindowrenderer.h"
#include <QWindow>

VulkanWindowRenderer::VulkanWindowRenderer(QWindow *window, Flags flags)
    : VulkanRenderer(flags),
      m_window(window),
      m_vulkanLib(QStringLiteral("vulkan-1"))
{
    if (!m_vulkanLib.load())
        qFatal("Failed to load vulkan-1");

    window->installEventFilter(this);
}

VulkanWindowRenderer::~VulkanWindowRenderer()
{
    m_vulkanLib.unload();
}

bool VulkanWindowRenderer::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::Expose) {
        if (m_window->isExposed()) {
            if (!m_inited)
                init();
        } else if (m_inited) {
            vkDeviceWaitIdle(m_vkDev);
            cleanup();
        }
    } else if (event->type() == QEvent::Resize) {
        if (m_inited && m_window->isExposed()) {
            vkDeviceWaitIdle(m_vkDev);
            recreateSwapChain();
        }
    }

    return false;
}

void VulkanWindowRenderer::init()
{
    if (m_inited)
        return;

    vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(m_vulkanLib.resolve("vkCreateInstance"));
    vkDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(m_vulkanLib.resolve("vkDestroyInstance"));
    vkEnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(m_vulkanLib.resolve("vkEnumeratePhysicalDevices"));
    vkGetPhysicalDeviceFeatures = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(m_vulkanLib.resolve("vkGetPhysicalDeviceFeatures"));
    vkGetPhysicalDeviceFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties>(m_vulkanLib.resolve("vkGetPhysicalDeviceFormatProperties"));
    vkGetPhysicalDeviceImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties>(m_vulkanLib.resolve("vkGetPhysicalDeviceImageFormatProperties"));
    vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(m_vulkanLib.resolve("vkGetPhysicalDeviceProperties"));
    vkGetPhysicalDeviceQueueFamilyProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(m_vulkanLib.resolve("vkGetPhysicalDeviceQueueFamilyProperties"));
    vkGetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(m_vulkanLib.resolve("vkGetPhysicalDeviceMemoryProperties"));
    vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(m_vulkanLib.resolve("vkGetInstanceProcAddr"));
    vkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(m_vulkanLib.resolve("vkGetDeviceProcAddr"));
    vkCreateDevice = reinterpret_cast<PFN_vkCreateDevice>(m_vulkanLib.resolve("vkCreateDevice"));
    vkDestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(m_vulkanLib.resolve("vkDestroyDevice"));
    vkEnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(m_vulkanLib.resolve("vkEnumerateInstanceExtensionProperties"));
    vkEnumerateDeviceExtensionProperties = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(m_vulkanLib.resolve("vkEnumerateDeviceExtensionProperties"));
    vkEnumerateInstanceLayerProperties = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(m_vulkanLib.resolve("vkEnumerateInstanceLayerProperties"));
    vkEnumerateDeviceLayerProperties = reinterpret_cast<PFN_vkEnumerateDeviceLayerProperties>(m_vulkanLib.resolve("vkEnumerateDeviceLayerProperties"));
    vkGetDeviceQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(m_vulkanLib.resolve("vkGetDeviceQueue"));
    vkQueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(m_vulkanLib.resolve("vkQueueSubmit"));
    vkQueueWaitIdle = reinterpret_cast<PFN_vkQueueWaitIdle>(m_vulkanLib.resolve("vkQueueWaitIdle"));
    vkDeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(m_vulkanLib.resolve("vkDeviceWaitIdle"));
    vkAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(m_vulkanLib.resolve("vkAllocateMemory"));
    vkFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(m_vulkanLib.resolve("vkFreeMemory"));
    vkMapMemory = reinterpret_cast<PFN_vkMapMemory>(m_vulkanLib.resolve("vkMapMemory"));
    vkUnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(m_vulkanLib.resolve("vkUnmapMemory"));
    vkFlushMappedMemoryRanges = reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(m_vulkanLib.resolve("vkFlushMappedMemoryRanges"));
    vkInvalidateMappedMemoryRanges = reinterpret_cast<PFN_vkInvalidateMappedMemoryRanges>(m_vulkanLib.resolve("vkInvalidateMappedMemoryRanges"));
    vkGetDeviceMemoryCommitment = reinterpret_cast<PFN_vkGetDeviceMemoryCommitment>(m_vulkanLib.resolve("vkGetDeviceMemoryCommitment"));
    vkBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(m_vulkanLib.resolve("vkBindBufferMemory"));
    vkBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(m_vulkanLib.resolve("vkBindImageMemory"));
    vkGetBufferMemoryRequirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(m_vulkanLib.resolve("vkGetBufferMemoryRequirements"));
    vkGetImageMemoryRequirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(m_vulkanLib.resolve("vkGetImageMemoryRequirements"));
    vkGetImageSparseMemoryRequirements = reinterpret_cast<PFN_vkGetImageSparseMemoryRequirements>(m_vulkanLib.resolve("vkGetImageSparseMemoryRequirements"));
    vkGetPhysicalDeviceSparseImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceSparseImageFormatProperties>(m_vulkanLib.resolve("vkGetPhysicalDeviceSparseImageFormatProperties"));
    vkQueueBindSparse = reinterpret_cast<PFN_vkQueueBindSparse>(m_vulkanLib.resolve("vkQueueBindSparse"));
    vkCreateFence = reinterpret_cast<PFN_vkCreateFence>(m_vulkanLib.resolve("vkCreateFence"));
    vkDestroyFence = reinterpret_cast<PFN_vkDestroyFence>(m_vulkanLib.resolve("vkDestroyFence"));
    vkResetFences = reinterpret_cast<PFN_vkResetFences>(m_vulkanLib.resolve("vkResetFences"));
    vkGetFenceStatus = reinterpret_cast<PFN_vkGetFenceStatus>(m_vulkanLib.resolve("vkGetFenceStatus"));
    vkWaitForFences = reinterpret_cast<PFN_vkWaitForFences>(m_vulkanLib.resolve("vkWaitForFences"));
    vkCreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(m_vulkanLib.resolve("vkCreateSemaphore"));
    vkDestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(m_vulkanLib.resolve("vkDestroySemaphore"));
    vkCreateEvent = reinterpret_cast<PFN_vkCreateEvent>(m_vulkanLib.resolve("vkCreateEvent"));
    vkDestroyEvent = reinterpret_cast<PFN_vkDestroyEvent>(m_vulkanLib.resolve("vkDestroyEvent"));
    vkGetEventStatus = reinterpret_cast<PFN_vkGetEventStatus>(m_vulkanLib.resolve("vkGetEventStatus"));
    vkSetEvent = reinterpret_cast<PFN_vkSetEvent>(m_vulkanLib.resolve("vkSetEvent"));
    vkResetEvent = reinterpret_cast<PFN_vkResetEvent>(m_vulkanLib.resolve("vkResetEvent"));
    vkCreateQueryPool = reinterpret_cast<PFN_vkCreateQueryPool>(m_vulkanLib.resolve("vkCreateQueryPool"));
    vkDestroyQueryPool = reinterpret_cast<PFN_vkDestroyQueryPool>(m_vulkanLib.resolve("vkDestroyQueryPool"));
    vkGetQueryPoolResults = reinterpret_cast<PFN_vkGetQueryPoolResults>(m_vulkanLib.resolve("vkGetQueryPoolResults"));
    vkCreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(m_vulkanLib.resolve("vkCreateBuffer"));
    vkDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(m_vulkanLib.resolve("vkDestroyBuffer"));
    vkCreateBufferView = reinterpret_cast<PFN_vkCreateBufferView>(m_vulkanLib.resolve("vkCreateBufferView"));
    vkDestroyBufferView = reinterpret_cast<PFN_vkDestroyBufferView>(m_vulkanLib.resolve("vkDestroyBufferView"));
    vkCreateImage = reinterpret_cast<PFN_vkCreateImage>(m_vulkanLib.resolve("vkCreateImage"));
    vkDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(m_vulkanLib.resolve("vkDestroyImage"));
    vkGetImageSubresourceLayout = reinterpret_cast<PFN_vkGetImageSubresourceLayout>(m_vulkanLib.resolve("vkGetImageSubresourceLayout"));
    vkCreateImageView = reinterpret_cast<PFN_vkCreateImageView>(m_vulkanLib.resolve("vkCreateImageView"));
    vkDestroyImageView = reinterpret_cast<PFN_vkDestroyImageView>(m_vulkanLib.resolve("vkDestroyImageView"));
    vkCreateShaderModule = reinterpret_cast<PFN_vkCreateShaderModule>(m_vulkanLib.resolve("vkCreateShaderModule"));
    vkDestroyShaderModule = reinterpret_cast<PFN_vkDestroyShaderModule>(m_vulkanLib.resolve("vkDestroyShaderModule"));
    vkCreatePipelineCache = reinterpret_cast<PFN_vkCreatePipelineCache>(m_vulkanLib.resolve("vkCreatePipelineCache"));
    vkDestroyPipelineCache = reinterpret_cast<PFN_vkDestroyPipelineCache>(m_vulkanLib.resolve("vkDestroyPipelineCache"));
    vkGetPipelineCacheData = reinterpret_cast<PFN_vkGetPipelineCacheData>(m_vulkanLib.resolve("vkGetPipelineCacheData"));
    vkMergePipelineCaches = reinterpret_cast<PFN_vkMergePipelineCaches>(m_vulkanLib.resolve("vkMergePipelineCaches"));
    vkCreateGraphicsPipelines = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(m_vulkanLib.resolve("vkCreateGraphicsPipelines"));
    vkCreateComputePipelines = reinterpret_cast<PFN_vkCreateComputePipelines>(m_vulkanLib.resolve("vkCreateComputePipelines"));
    vkDestroyPipeline = reinterpret_cast<PFN_vkDestroyPipeline>(m_vulkanLib.resolve("vkDestroyPipeline"));
    vkCreatePipelineLayout = reinterpret_cast<PFN_vkCreatePipelineLayout>(m_vulkanLib.resolve("vkCreatePipelineLayout"));
    vkDestroyPipelineLayout = reinterpret_cast<PFN_vkDestroyPipelineLayout>(m_vulkanLib.resolve("vkDestroyPipelineLayout"));
    vkCreateSampler = reinterpret_cast<PFN_vkCreateSampler>(m_vulkanLib.resolve("vkCreateSampler"));
    vkDestroySampler = reinterpret_cast<PFN_vkDestroySampler>(m_vulkanLib.resolve("vkDestroySampler"));
    vkCreateDescriptorSetLayout = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(m_vulkanLib.resolve("vkCreateDescriptorSetLayout"));
    vkDestroyDescriptorSetLayout = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(m_vulkanLib.resolve("vkDestroyDescriptorSetLayout"));
    vkCreateDescriptorPool = reinterpret_cast<PFN_vkCreateDescriptorPool>(m_vulkanLib.resolve("vkCreateDescriptorPool"));
    vkDestroyDescriptorPool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(m_vulkanLib.resolve("vkDestroyDescriptorPool"));
    vkResetDescriptorPool = reinterpret_cast<PFN_vkResetDescriptorPool>(m_vulkanLib.resolve("vkResetDescriptorPool"));
    vkAllocateDescriptorSets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(m_vulkanLib.resolve("vkAllocateDescriptorSets"));
    vkFreeDescriptorSets = reinterpret_cast<PFN_vkFreeDescriptorSets>(m_vulkanLib.resolve("vkFreeDescriptorSets"));
    vkUpdateDescriptorSets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(m_vulkanLib.resolve("vkUpdateDescriptorSets"));
    vkCreateFramebuffer = reinterpret_cast<PFN_vkCreateFramebuffer>(m_vulkanLib.resolve("vkCreateFramebuffer"));
    vkDestroyFramebuffer = reinterpret_cast<PFN_vkDestroyFramebuffer>(m_vulkanLib.resolve("vkDestroyFramebuffer"));
    vkCreateRenderPass = reinterpret_cast<PFN_vkCreateRenderPass>(m_vulkanLib.resolve("vkCreateRenderPass"));
    vkDestroyRenderPass = reinterpret_cast<PFN_vkDestroyRenderPass>(m_vulkanLib.resolve("vkDestroyRenderPass"));
    vkGetRenderAreaGranularity = reinterpret_cast<PFN_vkGetRenderAreaGranularity>(m_vulkanLib.resolve("vkGetRenderAreaGranularity"));
    vkCreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(m_vulkanLib.resolve("vkCreateCommandPool"));
    vkDestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(m_vulkanLib.resolve("vkDestroyCommandPool"));
    vkResetCommandPool = reinterpret_cast<PFN_vkResetCommandPool>(m_vulkanLib.resolve("vkResetCommandPool"));
    vkAllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(m_vulkanLib.resolve("vkAllocateCommandBuffers"));
    vkFreeCommandBuffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(m_vulkanLib.resolve("vkFreeCommandBuffers"));
    vkBeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(m_vulkanLib.resolve("vkBeginCommandBuffer"));
    vkEndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(m_vulkanLib.resolve("vkEndCommandBuffer"));
    vkResetCommandBuffer = reinterpret_cast<PFN_vkResetCommandBuffer>(m_vulkanLib.resolve("vkResetCommandBuffer"));
    vkCmdBindPipeline = reinterpret_cast<PFN_vkCmdBindPipeline>(m_vulkanLib.resolve("vkCmdBindPipeline"));
    vkCmdSetViewport = reinterpret_cast<PFN_vkCmdSetViewport>(m_vulkanLib.resolve("vkCmdSetViewport"));
    vkCmdSetScissor = reinterpret_cast<PFN_vkCmdSetScissor>(m_vulkanLib.resolve("vkCmdSetScissor"));
    vkCmdSetLineWidth = reinterpret_cast<PFN_vkCmdSetLineWidth>(m_vulkanLib.resolve("vkCmdSetLineWidth"));
    vkCmdSetDepthBias = reinterpret_cast<PFN_vkCmdSetDepthBias>(m_vulkanLib.resolve("vkCmdSetDepthBias"));
    vkCmdSetBlendConstants = reinterpret_cast<PFN_vkCmdSetBlendConstants>(m_vulkanLib.resolve("vkCmdSetBlendConstants"));
    vkCmdSetDepthBounds = reinterpret_cast<PFN_vkCmdSetDepthBounds>(m_vulkanLib.resolve("vkCmdSetDepthBounds"));
    vkCmdSetStencilCompareMask = reinterpret_cast<PFN_vkCmdSetStencilCompareMask>(m_vulkanLib.resolve("vkCmdSetStencilCompareMask"));
    vkCmdSetStencilWriteMask = reinterpret_cast<PFN_vkCmdSetStencilWriteMask>(m_vulkanLib.resolve("vkCmdSetStencilWriteMask"));
    vkCmdSetStencilReference = reinterpret_cast<PFN_vkCmdSetStencilReference>(m_vulkanLib.resolve("vkCmdSetStencilReference"));
    vkCmdBindDescriptorSets = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(m_vulkanLib.resolve("vkCmdBindDescriptorSets"));
    vkCmdBindIndexBuffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(m_vulkanLib.resolve("vkCmdBindIndexBuffer"));
    vkCmdBindVertexBuffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(m_vulkanLib.resolve("vkCmdBindVertexBuffers"));
    vkCmdDraw = reinterpret_cast<PFN_vkCmdDraw>(m_vulkanLib.resolve("vkCmdDraw"));
    vkCmdDrawIndexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(m_vulkanLib.resolve("vkCmdDrawIndexed"));
    vkCmdDrawIndirect = reinterpret_cast<PFN_vkCmdDrawIndirect>(m_vulkanLib.resolve("vkCmdDrawIndirect"));
    vkCmdDrawIndexedIndirect = reinterpret_cast<PFN_vkCmdDrawIndexedIndirect>(m_vulkanLib.resolve("vkCmdDrawIndexedIndirect"));
    vkCmdDispatch = reinterpret_cast<PFN_vkCmdDispatch>(m_vulkanLib.resolve("vkCmdDispatch"));
    vkCmdDispatchIndirect = reinterpret_cast<PFN_vkCmdDispatchIndirect>(m_vulkanLib.resolve("vkCmdDispatchIndirect"));
    vkCmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(m_vulkanLib.resolve("vkCmdCopyBuffer"));
    vkCmdCopyImage = reinterpret_cast<PFN_vkCmdCopyImage>(m_vulkanLib.resolve("vkCmdCopyImage"));
    vkCmdBlitImage = reinterpret_cast<PFN_vkCmdBlitImage>(m_vulkanLib.resolve("vkCmdBlitImage"));
    vkCmdCopyBufferToImage = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(m_vulkanLib.resolve("vkCmdCopyBufferToImage"));
    vkCmdCopyImageToBuffer = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(m_vulkanLib.resolve("vkCmdCopyImageToBuffer"));
    vkCmdUpdateBuffer = reinterpret_cast<PFN_vkCmdUpdateBuffer>(m_vulkanLib.resolve("vkCmdUpdateBuffer"));
    vkCmdFillBuffer = reinterpret_cast<PFN_vkCmdFillBuffer>(m_vulkanLib.resolve("vkCmdFillBuffer"));
    vkCmdClearColorImage = reinterpret_cast<PFN_vkCmdClearColorImage>(m_vulkanLib.resolve("vkCmdClearColorImage"));
    vkCmdClearDepthStencilImage = reinterpret_cast<PFN_vkCmdClearDepthStencilImage>(m_vulkanLib.resolve("vkCmdClearDepthStencilImage"));
    vkCmdClearAttachments = reinterpret_cast<PFN_vkCmdClearAttachments>(m_vulkanLib.resolve("vkCmdClearAttachments"));
    vkCmdResolveImage = reinterpret_cast<PFN_vkCmdResolveImage>(m_vulkanLib.resolve("vkCmdResolveImage"));
    vkCmdSetEvent = reinterpret_cast<PFN_vkCmdSetEvent>(m_vulkanLib.resolve("vkCmdSetEvent"));
    vkCmdResetEvent = reinterpret_cast<PFN_vkCmdResetEvent>(m_vulkanLib.resolve("vkCmdResetEvent"));
    vkCmdWaitEvents = reinterpret_cast<PFN_vkCmdWaitEvents>(m_vulkanLib.resolve("vkCmdWaitEvents"));
    vkCmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(m_vulkanLib.resolve("vkCmdPipelineBarrier"));
    vkCmdBeginQuery = reinterpret_cast<PFN_vkCmdBeginQuery>(m_vulkanLib.resolve("vkCmdBeginQuery"));
    vkCmdEndQuery = reinterpret_cast<PFN_vkCmdEndQuery>(m_vulkanLib.resolve("vkCmdEndQuery"));
    vkCmdResetQueryPool = reinterpret_cast<PFN_vkCmdResetQueryPool>(m_vulkanLib.resolve("vkCmdResetQueryPool"));
    vkCmdWriteTimestamp = reinterpret_cast<PFN_vkCmdWriteTimestamp>(m_vulkanLib.resolve("vkCmdWriteTimestamp"));
    vkCmdCopyQueryPoolResults = reinterpret_cast<PFN_vkCmdCopyQueryPoolResults>(m_vulkanLib.resolve("vkCmdCopyQueryPoolResults"));
    vkCmdPushConstants = reinterpret_cast<PFN_vkCmdPushConstants>(m_vulkanLib.resolve("vkCmdPushConstants"));
    vkCmdBeginRenderPass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(m_vulkanLib.resolve("vkCmdBeginRenderPass"));
    vkCmdNextSubpass = reinterpret_cast<PFN_vkCmdNextSubpass>(m_vulkanLib.resolve("vkCmdNextSubpass"));
    vkCmdEndRenderPass = reinterpret_cast<PFN_vkCmdEndRenderPass>(m_vulkanLib.resolve("vkCmdEndRenderPass"));
    vkCmdExecuteCommands = reinterpret_cast<PFN_vkCmdExecuteCommands>(m_vulkanLib.resolve("vkCmdExecuteCommands"));

    createDeviceAndSurface();
    recreateSwapChain();

    m_inited = true;
    qDebug("VK window renderer initialized");
}

void VulkanWindowRenderer::cleanup()
{
    if (!m_inited)
        return;

    qDebug("Stopping VK window renderer");
    m_inited = false;

    releaseDeviceAndSurface();
}

void VulkanWindowRenderer::createSurface()
{
    vkDestroySurfaceKHR = reinterpret_cast<PFN_vkDestroySurfaceKHR>(vkGetInstanceProcAddr(m_vkInst, "vkDestroySurfaceKHR"));
    vkGetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(vkGetInstanceProcAddr(m_vkInst, "vkGetPhysicalDeviceSurfaceSupportKHR"));
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(vkGetInstanceProcAddr(m_vkInst, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
    vkGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(vkGetInstanceProcAddr(m_vkInst, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
    vkGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(vkGetInstanceProcAddr(m_vkInst, "vkGetPhysicalDeviceSurfacePresentModesKHR"));

#ifdef Q_OS_WIN
    vkCreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(m_vkInst, "vkCreateWin32SurfaceKHR"));
    vkGetPhysicalDeviceWin32PresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>(vkGetInstanceProcAddr(m_vkInst, "vkGetPhysicalDeviceWin32PresentationSupportKHR"));

    VkWin32SurfaceCreateInfoKHR surfaceInfo;
    memset(&surfaceInfo, 0, sizeof(surfaceInfo));
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hinstance = GetModuleHandle(nullptr);
    surfaceInfo.hwnd = HWND(m_window->winId());
    VkResult err = vkCreateWin32SurfaceKHR(m_vkInst, &surfaceInfo, nullptr, &m_surface);
    if (err != VK_SUCCESS)
        qFatal("Failed to create Win32 surface: %d", err);
#endif
}

void VulkanWindowRenderer::releaseSurface()
{
    if (m_swapChain != VK_NULL_HANDLE) {
        for (uint32_t i = 0; i < m_swapChainBufferCount; ++i)
            vkDestroyImageView(m_vkDev, m_swapChainImageViews[i], nullptr);
        vkDestroySwapchainKHR(m_vkDev, m_swapChain, nullptr);
        m_swapChain = VK_NULL_HANDLE;
    }
    vkDestroySurfaceKHR(m_vkInst, m_surface, nullptr);
}

bool VulkanWindowRenderer::physicalDeviceSupportsPresent(int queueFamilyIdx)
{
    bool ok = false;
#ifdef Q_OS_WIN
    ok |= bool(vkGetPhysicalDeviceWin32PresentationSupportKHR(m_vkPhysDev, queueFamilyIdx));
#endif
    VkBool32 supported = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(m_vkPhysDev, queueFamilyIdx, m_surface, &supported);
    ok |= bool(supported);
    return ok;
}

void VulkanWindowRenderer::recreateSwapChain()
{
    Q_ASSERT(m_window);
    if (m_window->size().isEmpty())
        return;

    if (!vkCreateSwapchainKHR) {
        vkCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(vkGetDeviceProcAddr(m_vkDev, "vkCreateSwapchainKHR"));
        vkDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(vkGetDeviceProcAddr(m_vkDev, "vkDestroySwapchainKHR"));
        vkGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(vkGetDeviceProcAddr(m_vkDev, "vkGetSwapchainImagesKHR"));
        vkAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(m_vkDev, "vkAcquireNextImageKHR"));
        vkQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(vkGetDeviceProcAddr(m_vkDev, "vkQueuePresentKHR"));
    }

    VkColorSpaceKHR colorSpace = VkColorSpaceKHR(0);
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_vkPhysDev, m_surface, &formatCount, nullptr);
    if (formatCount) {
        QVector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_vkPhysDev, m_surface, &formatCount, formats.data());
        if (formats[0].format != VK_FORMAT_UNDEFINED) {
            m_colorFormat = formats[0].format;
            colorSpace = formats[0].colorSpace;
        }
    }

    VkSurfaceCapabilitiesKHR surfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_vkPhysDev, m_surface, &surfaceCaps);
    uint32_t reqBufferCount = 2;
    if (surfaceCaps.maxImageCount)
        reqBufferCount = qBound(surfaceCaps.minImageCount, reqBufferCount, surfaceCaps.maxImageCount);
    Q_ASSERT(surfaceCaps.minImageCount <= MAX_SWAPCHAIN_BUFFERS);
    reqBufferCount = qMin(reqBufferCount, MAX_SWAPCHAIN_BUFFERS);

    VkExtent2D bufferSize = surfaceCaps.currentExtent;
    if (bufferSize.width == uint32_t(-1))
        bufferSize.width = m_window->size().width();
    if (bufferSize.height == uint32_t(-1))
        bufferSize.height = m_window->size().height();

    VkSurfaceTransformFlagBitsKHR preTransform = surfaceCaps.currentTransform;
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    VkSwapchainKHR oldSwapChain = m_swapChain;
    VkSwapchainCreateInfoKHR swapChainInfo;
    memset(&swapChainInfo, 0, sizeof(swapChainInfo));
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.surface = m_surface;
    swapChainInfo.minImageCount = reqBufferCount;
    swapChainInfo.imageFormat = m_colorFormat;
    swapChainInfo.imageColorSpace = colorSpace;
    swapChainInfo.imageExtent = bufferSize;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainInfo.preTransform = preTransform;
    swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainInfo.presentMode = presentMode;
    swapChainInfo.clipped = true;
    swapChainInfo.oldSwapchain = oldSwapChain;

    qDebug("creating new swap chain of %d buffers, size %dx%d", reqBufferCount, bufferSize.width, bufferSize.height);

    VkResult err = vkCreateSwapchainKHR(m_vkDev, &swapChainInfo, nullptr, &m_swapChain);
    if (err != VK_SUCCESS)
        qFatal("Failed to create swap chain: %d", err);

    if (oldSwapChain != VK_NULL_HANDLE) {
        for (uint32_t i = 0; i < m_swapChainBufferCount; ++i)
            vkDestroyImageView(m_vkDev, m_swapChainImageViews[i], nullptr);
        vkDestroySwapchainKHR(m_vkDev, oldSwapChain, nullptr);
    }

    m_swapChainBufferCount = 0;
    err = vkGetSwapchainImagesKHR(m_vkDev, m_swapChain, &m_swapChainBufferCount, nullptr);
    if (err != VK_SUCCESS || m_swapChainBufferCount < 2)
        qFatal("Failed to get swapchain images: %d (count=%d)", err, m_swapChainBufferCount);

    err = vkGetSwapchainImagesKHR(m_vkDev, m_swapChain, &m_swapChainBufferCount, m_swapChainImages);
    if (err != VK_SUCCESS)
        qFatal("Failed to get swapchain images: %d", err);

    for (uint32_t i = 0; i < m_swapChainBufferCount; ++i) {
        VkImageViewCreateInfo imgViewInfo;
        memset(&imgViewInfo, 0, sizeof(imgViewInfo));
        imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imgViewInfo.image = m_swapChainImages[i];
        imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imgViewInfo.format = m_colorFormat;
        imgViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        imgViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        imgViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        imgViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgViewInfo.subresourceRange.levelCount = imgViewInfo.subresourceRange.layerCount = 1;
        err = vkCreateImageView(m_vkDev, &imgViewInfo, nullptr, &m_swapChainImageViews[i]);
        if (err != VK_SUCCESS)
            qFatal("Failed to create swapchain image view %d: %d", i, err);
    }
}
