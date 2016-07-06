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
#include <QElapsedTimer>
#include <QCoreApplication>

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
            m_window->requestUpdate();
        } else if (m_inited) {
            vkDeviceWaitIdle(m_vkDev);
            cleanup();
        }
    } else if (event->type() == QEvent::Resize) {
        if (m_inited && m_window->isExposed()) {
            vkDeviceWaitIdle(m_vkDev);
            recreateSwapChain();
        }
    } else if (event->type() == QEvent::UpdateRequest) {
        if (m_inited && m_window->isExposed()) {
            m_window->requestUpdate();
            if (beginFrame()) {
                renderFrame();
                endFrame();
            }
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

    for (uint32_t i = 0; i < MAX_SWAPCHAIN_BUFFERS; ++i)
        m_fb[i] = VK_NULL_HANDLE;

    for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; ++i) {
        m_frameCmdBuf[i] = VK_NULL_HANDLE;
        m_frameCmdBufRecording[i] = false;
        m_frameFence[i] = VK_NULL_HANDLE;
        m_acquireSem[i] = VK_NULL_HANDLE;
        m_renderSem[i] = VK_NULL_HANDLE;
    }
}

void VulkanWindowRenderer::releaseSurface()
{
    for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; ++i) {
        if (m_frameCmdBuf[i] != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(m_vkDev, m_vkCmdPool, 1, &m_frameCmdBuf[i]);
            m_frameCmdBuf[i] = VK_NULL_HANDLE;
        }
        if (m_frameFence[i] != VK_NULL_HANDLE) {
            vkDestroyFence(m_vkDev, m_frameFence[i], nullptr);
            m_frameFence[i] = VK_NULL_HANDLE;
        }
        if (m_acquireSem[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(m_vkDev, m_acquireSem[i], nullptr);
            m_acquireSem[i] = VK_NULL_HANDLE;
        }
        if (m_renderSem[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(m_vkDev, m_renderSem[i], nullptr);
            m_renderSem[i] = VK_NULL_HANDLE;
        }
    }

    if (m_renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_vkDev, m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }

    if (m_swapChain != VK_NULL_HANDLE) {
        for (uint32_t i = 0; i < m_swapChainBufferCount; ++i) {
            vkDestroyImageView(m_vkDev, m_swapChainImageViews[i], nullptr);
            if (m_fb[i] != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(m_vkDev, m_fb[i], nullptr);
                m_fb[i] = VK_NULL_HANDLE;
            }
        }
        vkDestroySwapchainKHR(m_vkDev, m_swapChain, nullptr);
        m_swapChain = VK_NULL_HANDLE;
        vkDestroyImageView(m_vkDev, m_dsView, nullptr);
        vkDestroyImage(m_vkDev, m_ds, nullptr);
        vkFreeMemory(m_vkDev, m_dsMem, nullptr);
        m_dsMem = VK_NULL_HANDLE;
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
    uint32_t reqBufferCount = REQUESTED_SWAPCHAIN_BUFFERS;
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

    qDebug("swap chain buffer count: %d", m_swapChainBufferCount);

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

    m_currentSwapChainBuffer = 0;
    m_currentFrame = 0;

    ensureFrameCmdBuf(m_currentFrame);

    for (uint32_t i = 0; i < m_swapChainBufferCount; ++i) {
        transitionImage(m_frameCmdBuf[m_currentFrame], m_swapChainImages[i],
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                        0, 0);
    }

    for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; ++i) {
        m_frameFenceActive[i] = false;
        if (m_frameFence[i] == VK_NULL_HANDLE) {
            VkFenceCreateInfo fenceInfo = {
                VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                nullptr,
                0
            };
            err = vkCreateFence(m_vkDev, &fenceInfo, nullptr, &m_frameFence[i]);
            if (err != VK_SUCCESS)
                qFatal("Failed to create fence: %d", err);
        }
        VkSemaphoreCreateInfo semInfo = {
            VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            nullptr,
            0
        };
        if (m_acquireSem[i] == VK_NULL_HANDLE) {
            err = vkCreateSemaphore(m_vkDev, &semInfo, nullptr, &m_acquireSem[i]);
            if (err != VK_SUCCESS)
                qFatal("Failed to create acquire semaphore: %d", err);
        }
        if (m_renderSem[i] == VK_NULL_HANDLE) {
            err = vkCreateSemaphore(m_vkDev, &semInfo, nullptr, &m_renderSem[i]);
            if (err != VK_SUCCESS)
                qFatal("Failed to create render semaphore: %d", err);
        }
    }

    if (m_dsMem != VK_NULL_HANDLE) {
        vkDestroyImageView(m_vkDev, m_dsView, nullptr);
        vkDestroyImage(m_vkDev, m_ds, nullptr);
        vkFreeMemory(m_vkDev, m_dsMem, nullptr);
    }

    VkImageCreateInfo imgInfo;
    memset(&imgInfo, 0, sizeof(imgInfo));
    imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.imageType = VK_IMAGE_TYPE_2D;
    imgInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
    imgInfo.extent.width = bufferSize.width;
    imgInfo.extent.height = bufferSize.height;
    imgInfo.mipLevels = 1;
    imgInfo.arrayLayers = 1;
    imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imgInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    err = vkCreateImage(m_vkDev, &imgInfo, nullptr, &m_ds);
    if (err != VK_SUCCESS)
        qFatal("Failed to create depth-stencil buffer: %d", err);

    VkMemoryRequirements dsMemReq;
    vkGetImageMemoryRequirements(m_vkDev, m_ds, &dsMemReq);
    uint memTypeIndex = 0;
    if (dsMemReq.memoryTypeBits)
        memTypeIndex = qCountTrailingZeroBits(dsMemReq.memoryTypeBits);

    VkMemoryAllocateInfo memInfo;
    memset(&memInfo, 0, sizeof(memInfo));
    memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memInfo.allocationSize = dsMemReq.size;
    memInfo.memoryTypeIndex = memTypeIndex;
    qDebug("allocating %lu bytes for depth-stencil", memInfo.allocationSize);

    err = vkAllocateMemory(m_vkDev, &memInfo, nullptr, &m_dsMem);
    if (err != VK_SUCCESS)
        qFatal("Failed to allocate depth-stencil memory: %d", err);

    err = vkBindImageMemory(m_vkDev, m_ds, m_dsMem, 0);
    if (err != VK_SUCCESS)
        qFatal("Failed to bind image memory for depth-stencil: %d", err);

    transitionImage(m_frameCmdBuf[m_currentFrame], m_ds,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, true);

    VkImageViewCreateInfo imgViewInfo;
    memset(&imgViewInfo, 0, sizeof(imgViewInfo));
    imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imgViewInfo.image = m_ds;
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
    imgViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    imgViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    imgViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    imgViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    imgViewInfo.subresourceRange.levelCount = imgViewInfo.subresourceRange.layerCount = 1;
    err = vkCreateImageView(m_vkDev, &imgViewInfo, nullptr, &m_dsView);
    if (err != VK_SUCCESS)
        qFatal("Failed to create depth-stencil view: %d", err);

    if (m_renderPass == VK_NULL_HANDLE) {
        VkAttachmentDescription attDesc[2];
        memset(attDesc, 0, sizeof(attDesc));
        attDesc[0].format = m_colorFormat;
        attDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attDesc[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attDesc[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attDesc[1].format = VK_FORMAT_D24_UNORM_S8_UINT;
        attDesc[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attDesc[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attDesc[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // do not write out depth
        attDesc[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
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
        VkResult err = vkCreateRenderPass(m_vkDev, &rpInfo, nullptr, &m_renderPass);
        if (err != VK_SUCCESS)
            qFatal("Failed to create renderpass: %d", err);
    }

    for (uint32_t i = 0; i < m_swapChainBufferCount; ++i) {
        if (m_fb[i] != VK_NULL_HANDLE)
            vkDestroyFramebuffer(m_vkDev, m_fb[i], nullptr);
        VkImageView views[2] = { m_swapChainImageViews[i], m_dsView };
        VkFramebufferCreateInfo fbInfo;
        memset(&fbInfo, 0, sizeof(fbInfo));
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = m_renderPass;
        fbInfo.attachmentCount = 2;
        fbInfo.pAttachments = views;
        fbInfo.width = bufferSize.width;
        fbInfo.height = bufferSize.height;
        fbInfo.layers = 1;
        err = vkCreateFramebuffer(m_vkDev, &fbInfo, nullptr, &m_fb[i]);
        if (err != VK_SUCCESS)
            qFatal("Failed to create framebuffer: %d", err);
    }
}

void VulkanWindowRenderer::ensureFrameCmdBuf(int frame)
{
    if (m_frameCmdBuf[frame] != VK_NULL_HANDLE) {
        if (m_frameCmdBufRecording[frame])
            return;
        vkFreeCommandBuffers(m_vkDev, m_vkCmdPool, 1, &m_frameCmdBuf[frame]);
    }

    VkCommandBufferAllocateInfo cmdBufInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, m_vkCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1
    };
    VkResult err = vkAllocateCommandBuffers(m_vkDev, &cmdBufInfo, &m_frameCmdBuf[frame]);
    if (err != VK_SUCCESS)
        qFatal("Failed to allocate frame command buffer: %d", err);

    VkCommandBufferBeginInfo cmdBufBeginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, 0, nullptr };
    err = vkBeginCommandBuffer(m_frameCmdBuf[frame], &cmdBufBeginInfo);
    if (err != VK_SUCCESS)
        qFatal("Failed to begin frame command buffer: %d", err);

    m_frameCmdBufRecording[frame] = true;
}

bool VulkanWindowRenderer::beginFrame()
{
    if (m_frameFenceActive[m_currentFrame]) {
        qDebug("wait fence %p", m_frameFence[m_currentFrame]);
        vkWaitForFences(m_vkDev, 1, &m_frameFence[m_currentFrame], true, UINT64_MAX);
        vkResetFences(m_vkDev, 1, &m_frameFence[m_currentFrame]);
    }

    VkResult err = vkAcquireNextImageKHR(m_vkDev, m_swapChain, UINT64_MAX,
                                         m_acquireSem[m_currentFrame], VK_NULL_HANDLE,
                                         &m_currentSwapChainBuffer);
    if (err != VK_SUCCESS) {
        if (err == VK_ERROR_OUT_OF_DATE_KHR) {
            qDebug("out of date in acquire");
            vkDeviceWaitIdle(m_vkDev);
            recreateSwapChain();
            return false;
        } else if (err != VK_SUBOPTIMAL_KHR) {
            qWarning("Failed to acquire next swapchain image: %d", err);
            return false;
        }
    }

    qDebug("current swapchain buffer is %d, current frame is %d", m_currentSwapChainBuffer, m_currentFrame);

    m_frameFenceActive[m_currentFrame] = true;
    ensureFrameCmdBuf(m_currentFrame);

    transitionImage(m_frameCmdBuf[m_currentFrame], m_swapChainImages[m_currentSwapChainBuffer],
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

    return true;
}

void VulkanWindowRenderer::endFrame()
{
    transitionImage(m_frameCmdBuf[m_currentFrame], m_swapChainImages[m_currentSwapChainBuffer],
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0);

    VkResult err = vkEndCommandBuffer(m_frameCmdBuf[m_currentFrame]);
    if (err != VK_SUCCESS)
        qFatal("Failed to end frame command buffer: %d", err);
    m_frameCmdBufRecording[m_currentFrame] = false;

    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(submitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_frameCmdBuf[m_currentFrame];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_acquireSem[m_currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_renderSem[m_currentFrame];
    VkPipelineStageFlags psf = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    submitInfo.pWaitDstStageMask = &psf;
    err = vkQueueSubmit(m_vkQueue, 1, &submitInfo, m_frameFence[m_currentFrame]);
    if (err != VK_SUCCESS) {
        qWarning("Failed to submit to command queue: %d", err);
        return;
    }

    VkPresentInfoKHR presInfo;
    memset(&presInfo, 0, sizeof(presInfo));
    presInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presInfo.swapchainCount = 1;
    presInfo.pSwapchains = &m_swapChain;
    presInfo.pImageIndices = &m_currentSwapChainBuffer;
    presInfo.waitSemaphoreCount = 1;
    presInfo.pWaitSemaphores = &m_renderSem[m_currentFrame];

    err = vkQueuePresentKHR(m_vkQueue, &presInfo);
    if (err != VK_SUCCESS) {
        if (err == VK_ERROR_OUT_OF_DATE_KHR) {
            qDebug("out of date in present");
            vkDeviceWaitIdle(m_vkDev);
            recreateSwapChain();
            return;
        } else if (err != VK_SUBOPTIMAL_KHR) {
            qWarning("Failed to present: %d", err);
        }
    }

    m_currentFrame = (m_currentFrame + 1) % FRAMES_IN_FLIGHT;
}

QElapsedTimer t;

void VulkanWindowRenderer::renderFrame()
{
    qDebug("renderframe %d: elapsed %lld", m_currentFrame, t.restart());

    Q_ASSERT(m_frameCmdBufRecording[m_currentFrame]);
    VkCommandBuffer &cb = m_frameCmdBuf[m_currentFrame];
    VkImage &img = m_swapChainImages[m_currentSwapChainBuffer];
    VkFramebuffer &fb = m_fb[m_currentSwapChainBuffer];
    const QSize size = m_window->size();

    VkClearColorValue clearColor = { 0.0f, m_g, 0.0f, 1.0f };
    VkImageSubresourceRange subResRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    vkCmdClearColorImage(cb, img, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subResRange);
    m_g += 0.01f;
    if (m_g > 1.0f)
        m_g = 0.0f;

//    VkRenderPassBeginInfo rpBeginInfo;
//    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//    rpBeginInfo.pNext = nullptr;
//    rpBeginInfo.renderPass = m_renderPass;
//    rpBeginInfo.framebuffer = fb;
//    rpBeginInfo.renderArea = qtvk_rect2D(QRect(QPoint(0, 0), size));
//    rpBeginInfo.clearValueCount = 1;
//    VkClearColorValue clearColor = { 0.0f, 1.0f, 0.0f, 1.0f };
//    VkClearValue clearValue;
//    clearValue.color = clearColor;
//    clearValue.depthStencil.depth = 1.0f;
//    clearValue.depthStencil.stencil = 0;
//    rpBeginInfo.pClearValues = &clearValue;

//    vkCmdBeginRenderPass(cb, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

//    VkViewport viewport = { 0, 0, float(size.width()), float(size.height()), 0, 1 };
//    vkCmdSetViewport(cb, 0, 1, &viewport);

//    vkCmdEndRenderPass(cb);
}
