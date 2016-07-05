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
#include <QLibrary>

VulkanWindowRenderer::VulkanWindowRenderer(QWindow *window)
    : VulkanRenderer(window),
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
            cleanup();
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

    createDevice();

    m_inited = true;
    qDebug("VK window renderer initialized");
}

void VulkanWindowRenderer::cleanup()
{
    if (!m_inited)
        return;

    qDebug("Stopping VK window renderer");
    m_inited = false;

    releaseRenderTarget();

    releaseDevice();
}
