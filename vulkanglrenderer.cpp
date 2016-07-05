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

#include "vulkanglrenderer.h"
#include <QQuickWindow>
#include <QOpenGLContext>

VulkanGLRenderer::VulkanGLRenderer(QQuickWindow *window)
// 'window' in the base class is left null, meaning all surface and swapchain stuff is skipped
    : m_quickWindow(window)
{
    connect(window, &QQuickWindow::beforeRendering, this, &VulkanGLRenderer::onBeforeGLRendering, Qt::DirectConnection);
    connect(window, &QQuickWindow::sceneGraphInvalidated, this, &VulkanGLRenderer::onInvalidate, Qt::DirectConnection);
    connect(window, &QQuickWindow::sceneGraphAboutToStop, this, &VulkanGLRenderer::onInvalidate, Qt::DirectConnection);
}

void VulkanGLRenderer::init()
{
    if (m_inited)
        return;

    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx)
        qFatal("No OpenGL context");

    if (!ctx->hasExtension("GL_NV_draw_vulkan_image"))
        qFatal("This example requires the GL_NV_draw_vulkan_image extension");

    glGetVkProcAddrNV = reinterpret_cast<PFN_glGetVkProcAddrNV>(ctx->getProcAddress("glGetVkProcAddrNV"));
    glWaitVkSemaphoreNV = reinterpret_cast<PFN_glWaitVkSemaphoreNV>(ctx->getProcAddress("glWaitVkSemaphoreNV"));
    glSignalVkSemaphoreNV = reinterpret_cast<PFN_glSignalVkSemaphoreNV>(ctx->getProcAddress("glSignalVkSemaphoreNV"));
    glSignalVkFenceNV = reinterpret_cast<PFN_glSignalVkFenceNV>(ctx->getProcAddress("glSignalVkFenceNV"));
    glDrawVkImageNV = reinterpret_cast<PFN_glDrawVkImageNV>(ctx->getProcAddress("glDrawVkImageNV"));

    if (!glGetVkProcAddrNV || !glWaitVkSemaphoreNV || !glSignalVkSemaphoreNV || !glSignalVkFenceNV || !glDrawVkImageNV)
        qFatal("Could not find GL_NV_draw_vulkan_image entry points");

    vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(glGetVkProcAddrNV("vkCreateInstance"));
    vkDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(glGetVkProcAddrNV("vkDestroyInstance"));
    vkEnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(glGetVkProcAddrNV("vkEnumeratePhysicalDevices"));
    vkGetPhysicalDeviceFeatures = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(glGetVkProcAddrNV("vkGetPhysicalDeviceFeatures"));
    vkGetPhysicalDeviceFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties>(glGetVkProcAddrNV("vkGetPhysicalDeviceFormatProperties"));
    vkGetPhysicalDeviceImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties>(glGetVkProcAddrNV("vkGetPhysicalDeviceImageFormatProperties"));
    vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(glGetVkProcAddrNV("vkGetPhysicalDeviceProperties"));
    vkGetPhysicalDeviceQueueFamilyProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(glGetVkProcAddrNV("vkGetPhysicalDeviceQueueFamilyProperties"));
    vkGetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(glGetVkProcAddrNV("vkGetPhysicalDeviceMemoryProperties"));
    vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(glGetVkProcAddrNV("vkGetInstanceProcAddr"));
    vkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(glGetVkProcAddrNV("vkGetDeviceProcAddr"));
    vkCreateDevice = reinterpret_cast<PFN_vkCreateDevice>(glGetVkProcAddrNV("vkCreateDevice"));
    vkDestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(glGetVkProcAddrNV("vkDestroyDevice"));
    vkEnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(glGetVkProcAddrNV("vkEnumerateInstanceExtensionProperties"));
    vkEnumerateDeviceExtensionProperties = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(glGetVkProcAddrNV("vkEnumerateDeviceExtensionProperties"));
    vkEnumerateInstanceLayerProperties = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(glGetVkProcAddrNV("vkEnumerateInstanceLayerProperties"));
    vkEnumerateDeviceLayerProperties = reinterpret_cast<PFN_vkEnumerateDeviceLayerProperties>(glGetVkProcAddrNV("vkEnumerateDeviceLayerProperties"));
    vkGetDeviceQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(glGetVkProcAddrNV("vkGetDeviceQueue"));
    vkQueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(glGetVkProcAddrNV("vkQueueSubmit"));
    vkQueueWaitIdle = reinterpret_cast<PFN_vkQueueWaitIdle>(glGetVkProcAddrNV("vkQueueWaitIdle"));
    vkDeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(glGetVkProcAddrNV("vkDeviceWaitIdle"));
    vkAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(glGetVkProcAddrNV("vkAllocateMemory"));
    vkFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(glGetVkProcAddrNV("vkFreeMemory"));
    vkMapMemory = reinterpret_cast<PFN_vkMapMemory>(glGetVkProcAddrNV("vkMapMemory"));
    vkUnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(glGetVkProcAddrNV("vkUnmapMemory"));
    vkFlushMappedMemoryRanges = reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(glGetVkProcAddrNV("vkFlushMappedMemoryRanges"));
    vkInvalidateMappedMemoryRanges = reinterpret_cast<PFN_vkInvalidateMappedMemoryRanges>(glGetVkProcAddrNV("vkInvalidateMappedMemoryRanges"));
    vkGetDeviceMemoryCommitment = reinterpret_cast<PFN_vkGetDeviceMemoryCommitment>(glGetVkProcAddrNV("vkGetDeviceMemoryCommitment"));
    vkBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(glGetVkProcAddrNV("vkBindBufferMemory"));
    vkBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(glGetVkProcAddrNV("vkBindImageMemory"));
    vkGetBufferMemoryRequirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(glGetVkProcAddrNV("vkGetBufferMemoryRequirements"));
    vkGetImageMemoryRequirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(glGetVkProcAddrNV("vkGetImageMemoryRequirements"));
    vkGetImageSparseMemoryRequirements = reinterpret_cast<PFN_vkGetImageSparseMemoryRequirements>(glGetVkProcAddrNV("vkGetImageSparseMemoryRequirements"));
    vkGetPhysicalDeviceSparseImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceSparseImageFormatProperties>(glGetVkProcAddrNV("vkGetPhysicalDeviceSparseImageFormatProperties"));
    vkQueueBindSparse = reinterpret_cast<PFN_vkQueueBindSparse>(glGetVkProcAddrNV("vkQueueBindSparse"));
    vkCreateFence = reinterpret_cast<PFN_vkCreateFence>(glGetVkProcAddrNV("vkCreateFence"));
    vkDestroyFence = reinterpret_cast<PFN_vkDestroyFence>(glGetVkProcAddrNV("vkDestroyFence"));
    vkResetFences = reinterpret_cast<PFN_vkResetFences>(glGetVkProcAddrNV("vkResetFences"));
    vkGetFenceStatus = reinterpret_cast<PFN_vkGetFenceStatus>(glGetVkProcAddrNV("vkGetFenceStatus"));
    vkWaitForFences = reinterpret_cast<PFN_vkWaitForFences>(glGetVkProcAddrNV("vkWaitForFences"));
    vkCreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(glGetVkProcAddrNV("vkCreateSemaphore"));
    vkDestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(glGetVkProcAddrNV("vkDestroySemaphore"));
    vkCreateEvent = reinterpret_cast<PFN_vkCreateEvent>(glGetVkProcAddrNV("vkCreateEvent"));
    vkDestroyEvent = reinterpret_cast<PFN_vkDestroyEvent>(glGetVkProcAddrNV("vkDestroyEvent"));
    vkGetEventStatus = reinterpret_cast<PFN_vkGetEventStatus>(glGetVkProcAddrNV("vkGetEventStatus"));
    vkSetEvent = reinterpret_cast<PFN_vkSetEvent>(glGetVkProcAddrNV("vkSetEvent"));
    vkResetEvent = reinterpret_cast<PFN_vkResetEvent>(glGetVkProcAddrNV("vkResetEvent"));
    vkCreateQueryPool = reinterpret_cast<PFN_vkCreateQueryPool>(glGetVkProcAddrNV("vkCreateQueryPool"));
    vkDestroyQueryPool = reinterpret_cast<PFN_vkDestroyQueryPool>(glGetVkProcAddrNV("vkDestroyQueryPool"));
    vkGetQueryPoolResults = reinterpret_cast<PFN_vkGetQueryPoolResults>(glGetVkProcAddrNV("vkGetQueryPoolResults"));
    vkCreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(glGetVkProcAddrNV("vkCreateBuffer"));
    vkDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(glGetVkProcAddrNV("vkDestroyBuffer"));
    vkCreateBufferView = reinterpret_cast<PFN_vkCreateBufferView>(glGetVkProcAddrNV("vkCreateBufferView"));
    vkDestroyBufferView = reinterpret_cast<PFN_vkDestroyBufferView>(glGetVkProcAddrNV("vkDestroyBufferView"));
    vkCreateImage = reinterpret_cast<PFN_vkCreateImage>(glGetVkProcAddrNV("vkCreateImage"));
    vkDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(glGetVkProcAddrNV("vkDestroyImage"));
    vkGetImageSubresourceLayout = reinterpret_cast<PFN_vkGetImageSubresourceLayout>(glGetVkProcAddrNV("vkGetImageSubresourceLayout"));
    vkCreateImageView = reinterpret_cast<PFN_vkCreateImageView>(glGetVkProcAddrNV("vkCreateImageView"));
    vkDestroyImageView = reinterpret_cast<PFN_vkDestroyImageView>(glGetVkProcAddrNV("vkDestroyImageView"));
    vkCreateShaderModule = reinterpret_cast<PFN_vkCreateShaderModule>(glGetVkProcAddrNV("vkCreateShaderModule"));
    vkDestroyShaderModule = reinterpret_cast<PFN_vkDestroyShaderModule>(glGetVkProcAddrNV("vkDestroyShaderModule"));
    vkCreatePipelineCache = reinterpret_cast<PFN_vkCreatePipelineCache>(glGetVkProcAddrNV("vkCreatePipelineCache"));
    vkDestroyPipelineCache = reinterpret_cast<PFN_vkDestroyPipelineCache>(glGetVkProcAddrNV("vkDestroyPipelineCache"));
    vkGetPipelineCacheData = reinterpret_cast<PFN_vkGetPipelineCacheData>(glGetVkProcAddrNV("vkGetPipelineCacheData"));
    vkMergePipelineCaches = reinterpret_cast<PFN_vkMergePipelineCaches>(glGetVkProcAddrNV("vkMergePipelineCaches"));
    vkCreateGraphicsPipelines = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(glGetVkProcAddrNV("vkCreateGraphicsPipelines"));
    vkCreateComputePipelines = reinterpret_cast<PFN_vkCreateComputePipelines>(glGetVkProcAddrNV("vkCreateComputePipelines"));
    vkDestroyPipeline = reinterpret_cast<PFN_vkDestroyPipeline>(glGetVkProcAddrNV("vkDestroyPipeline"));
    vkCreatePipelineLayout = reinterpret_cast<PFN_vkCreatePipelineLayout>(glGetVkProcAddrNV("vkCreatePipelineLayout"));
    vkDestroyPipelineLayout = reinterpret_cast<PFN_vkDestroyPipelineLayout>(glGetVkProcAddrNV("vkDestroyPipelineLayout"));
    vkCreateSampler = reinterpret_cast<PFN_vkCreateSampler>(glGetVkProcAddrNV("vkCreateSampler"));
    vkDestroySampler = reinterpret_cast<PFN_vkDestroySampler>(glGetVkProcAddrNV("vkDestroySampler"));
    vkCreateDescriptorSetLayout = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(glGetVkProcAddrNV("vkCreateDescriptorSetLayout"));
    vkDestroyDescriptorSetLayout = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(glGetVkProcAddrNV("vkDestroyDescriptorSetLayout"));
    vkCreateDescriptorPool = reinterpret_cast<PFN_vkCreateDescriptorPool>(glGetVkProcAddrNV("vkCreateDescriptorPool"));
    vkDestroyDescriptorPool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(glGetVkProcAddrNV("vkDestroyDescriptorPool"));
    vkResetDescriptorPool = reinterpret_cast<PFN_vkResetDescriptorPool>(glGetVkProcAddrNV("vkResetDescriptorPool"));
    vkAllocateDescriptorSets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(glGetVkProcAddrNV("vkAllocateDescriptorSets"));
    vkFreeDescriptorSets = reinterpret_cast<PFN_vkFreeDescriptorSets>(glGetVkProcAddrNV("vkFreeDescriptorSets"));
    vkUpdateDescriptorSets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(glGetVkProcAddrNV("vkUpdateDescriptorSets"));
    vkCreateFramebuffer = reinterpret_cast<PFN_vkCreateFramebuffer>(glGetVkProcAddrNV("vkCreateFramebuffer"));
    vkDestroyFramebuffer = reinterpret_cast<PFN_vkDestroyFramebuffer>(glGetVkProcAddrNV("vkDestroyFramebuffer"));
    vkCreateRenderPass = reinterpret_cast<PFN_vkCreateRenderPass>(glGetVkProcAddrNV("vkCreateRenderPass"));
    vkDestroyRenderPass = reinterpret_cast<PFN_vkDestroyRenderPass>(glGetVkProcAddrNV("vkDestroyRenderPass"));
    vkGetRenderAreaGranularity = reinterpret_cast<PFN_vkGetRenderAreaGranularity>(glGetVkProcAddrNV("vkGetRenderAreaGranularity"));
    vkCreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(glGetVkProcAddrNV("vkCreateCommandPool"));
    vkDestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(glGetVkProcAddrNV("vkDestroyCommandPool"));
    vkResetCommandPool = reinterpret_cast<PFN_vkResetCommandPool>(glGetVkProcAddrNV("vkResetCommandPool"));
    vkAllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(glGetVkProcAddrNV("vkAllocateCommandBuffers"));
    vkFreeCommandBuffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(glGetVkProcAddrNV("vkFreeCommandBuffers"));
    vkBeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(glGetVkProcAddrNV("vkBeginCommandBuffer"));
    vkEndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(glGetVkProcAddrNV("vkEndCommandBuffer"));
    vkResetCommandBuffer = reinterpret_cast<PFN_vkResetCommandBuffer>(glGetVkProcAddrNV("vkResetCommandBuffer"));
    vkCmdBindPipeline = reinterpret_cast<PFN_vkCmdBindPipeline>(glGetVkProcAddrNV("vkCmdBindPipeline"));
    vkCmdSetViewport = reinterpret_cast<PFN_vkCmdSetViewport>(glGetVkProcAddrNV("vkCmdSetViewport"));
    vkCmdSetScissor = reinterpret_cast<PFN_vkCmdSetScissor>(glGetVkProcAddrNV("vkCmdSetScissor"));
    vkCmdSetLineWidth = reinterpret_cast<PFN_vkCmdSetLineWidth>(glGetVkProcAddrNV("vkCmdSetLineWidth"));
    vkCmdSetDepthBias = reinterpret_cast<PFN_vkCmdSetDepthBias>(glGetVkProcAddrNV("vkCmdSetDepthBias"));
    vkCmdSetBlendConstants = reinterpret_cast<PFN_vkCmdSetBlendConstants>(glGetVkProcAddrNV("vkCmdSetBlendConstants"));
    vkCmdSetDepthBounds = reinterpret_cast<PFN_vkCmdSetDepthBounds>(glGetVkProcAddrNV("vkCmdSetDepthBounds"));
    vkCmdSetStencilCompareMask = reinterpret_cast<PFN_vkCmdSetStencilCompareMask>(glGetVkProcAddrNV("vkCmdSetStencilCompareMask"));
    vkCmdSetStencilWriteMask = reinterpret_cast<PFN_vkCmdSetStencilWriteMask>(glGetVkProcAddrNV("vkCmdSetStencilWriteMask"));
    vkCmdSetStencilReference = reinterpret_cast<PFN_vkCmdSetStencilReference>(glGetVkProcAddrNV("vkCmdSetStencilReference"));
    vkCmdBindDescriptorSets = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(glGetVkProcAddrNV("vkCmdBindDescriptorSets"));
    vkCmdBindIndexBuffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(glGetVkProcAddrNV("vkCmdBindIndexBuffer"));
    vkCmdBindVertexBuffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(glGetVkProcAddrNV("vkCmdBindVertexBuffers"));
    vkCmdDraw = reinterpret_cast<PFN_vkCmdDraw>(glGetVkProcAddrNV("vkCmdDraw"));
    vkCmdDrawIndexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(glGetVkProcAddrNV("vkCmdDrawIndexed"));
    vkCmdDrawIndirect = reinterpret_cast<PFN_vkCmdDrawIndirect>(glGetVkProcAddrNV("vkCmdDrawIndirect"));
    vkCmdDrawIndexedIndirect = reinterpret_cast<PFN_vkCmdDrawIndexedIndirect>(glGetVkProcAddrNV("vkCmdDrawIndexedIndirect"));
    vkCmdDispatch = reinterpret_cast<PFN_vkCmdDispatch>(glGetVkProcAddrNV("vkCmdDispatch"));
    vkCmdDispatchIndirect = reinterpret_cast<PFN_vkCmdDispatchIndirect>(glGetVkProcAddrNV("vkCmdDispatchIndirect"));
    vkCmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(glGetVkProcAddrNV("vkCmdCopyBuffer"));
    vkCmdCopyImage = reinterpret_cast<PFN_vkCmdCopyImage>(glGetVkProcAddrNV("vkCmdCopyImage"));
    vkCmdBlitImage = reinterpret_cast<PFN_vkCmdBlitImage>(glGetVkProcAddrNV("vkCmdBlitImage"));
    vkCmdCopyBufferToImage = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(glGetVkProcAddrNV("vkCmdCopyBufferToImage"));
    vkCmdCopyImageToBuffer = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(glGetVkProcAddrNV("vkCmdCopyImageToBuffer"));
    vkCmdUpdateBuffer = reinterpret_cast<PFN_vkCmdUpdateBuffer>(glGetVkProcAddrNV("vkCmdUpdateBuffer"));
    vkCmdFillBuffer = reinterpret_cast<PFN_vkCmdFillBuffer>(glGetVkProcAddrNV("vkCmdFillBuffer"));
    vkCmdClearColorImage = reinterpret_cast<PFN_vkCmdClearColorImage>(glGetVkProcAddrNV("vkCmdClearColorImage"));
    vkCmdClearDepthStencilImage = reinterpret_cast<PFN_vkCmdClearDepthStencilImage>(glGetVkProcAddrNV("vkCmdClearDepthStencilImage"));
    vkCmdClearAttachments = reinterpret_cast<PFN_vkCmdClearAttachments>(glGetVkProcAddrNV("vkCmdClearAttachments"));
    vkCmdResolveImage = reinterpret_cast<PFN_vkCmdResolveImage>(glGetVkProcAddrNV("vkCmdResolveImage"));
    vkCmdSetEvent = reinterpret_cast<PFN_vkCmdSetEvent>(glGetVkProcAddrNV("vkCmdSetEvent"));
    vkCmdResetEvent = reinterpret_cast<PFN_vkCmdResetEvent>(glGetVkProcAddrNV("vkCmdResetEvent"));
    vkCmdWaitEvents = reinterpret_cast<PFN_vkCmdWaitEvents>(glGetVkProcAddrNV("vkCmdWaitEvents"));
    vkCmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(glGetVkProcAddrNV("vkCmdPipelineBarrier"));
    vkCmdBeginQuery = reinterpret_cast<PFN_vkCmdBeginQuery>(glGetVkProcAddrNV("vkCmdBeginQuery"));
    vkCmdEndQuery = reinterpret_cast<PFN_vkCmdEndQuery>(glGetVkProcAddrNV("vkCmdEndQuery"));
    vkCmdResetQueryPool = reinterpret_cast<PFN_vkCmdResetQueryPool>(glGetVkProcAddrNV("vkCmdResetQueryPool"));
    vkCmdWriteTimestamp = reinterpret_cast<PFN_vkCmdWriteTimestamp>(glGetVkProcAddrNV("vkCmdWriteTimestamp"));
    vkCmdCopyQueryPoolResults = reinterpret_cast<PFN_vkCmdCopyQueryPoolResults>(glGetVkProcAddrNV("vkCmdCopyQueryPoolResults"));
    vkCmdPushConstants = reinterpret_cast<PFN_vkCmdPushConstants>(glGetVkProcAddrNV("vkCmdPushConstants"));
    vkCmdBeginRenderPass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(glGetVkProcAddrNV("vkCmdBeginRenderPass"));
    vkCmdNextSubpass = reinterpret_cast<PFN_vkCmdNextSubpass>(glGetVkProcAddrNV("vkCmdNextSubpass"));
    vkCmdEndRenderPass = reinterpret_cast<PFN_vkCmdEndRenderPass>(glGetVkProcAddrNV("vkCmdEndRenderPass"));
    vkCmdExecuteCommands = reinterpret_cast<PFN_vkCmdExecuteCommands>(glGetVkProcAddrNV("vkCmdExecuteCommands"));

    createDevice();

    VkSemaphoreCreateInfo semInfo;
    memset(&semInfo, 0, sizeof(semInfo));
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult err = vkCreateSemaphore(m_vkDev, &semInfo, nullptr, &m_semRender);
    if (err != VK_SUCCESS)
        qFatal("Failed to create render semaphore");

    memset(&semInfo, 0, sizeof(semInfo));
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_vkDev, &semInfo, nullptr, &m_semPresent);
    if (err != VK_SUCCESS)
        qFatal("Failed to create present semaphore");

    m_inited = true;
    qDebug("VK-on-GL renderer initialized");
}

void VulkanGLRenderer::onInvalidate()
{
    if (!m_inited)
        return;

    qDebug("Stopping VK-on-GL renderer");
    m_inited = false;

    releaseRenderTarget();
    m_lastWindowSize = QSize();

    vkDestroySemaphore(m_vkDev, m_semRender, nullptr);
    vkDestroySemaphore(m_vkDev, m_semPresent, nullptr);

    releaseDevice();
}

void VulkanGLRenderer::onBeforeGLRendering()
{
    init();

    const QSize windowSize = m_quickWindow->size();
    if (windowSize != m_lastWindowSize) {
        vkDeviceWaitIdle(m_vkDev);
        releaseRenderTarget();
        m_lastWindowSize = windowSize;
        if (windowSize.isEmpty())
            return;
        qDebug("resize %dx%d", windowSize.width(), windowSize.height());
        createRenderTarget(windowSize);
    }

    render(windowSize);

    present();
}

void VulkanGLRenderer::present()
{
    if (m_lastWindowSize.isEmpty())
        return;

    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(submitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_semRender;
    vkQueueSubmit(m_vkQueue, 1, &submitInfo, VK_NULL_HANDLE);

    glWaitVkSemaphoreNV(reinterpret_cast<GLuint64>(m_semRender));
    glDrawVkImageNV(reinterpret_cast<GLuint64>(m_color), 0,
                    0, 0, m_lastWindowSize.width(), m_lastWindowSize.height(), 0,
                    0, 1, 1, 0);
    glSignalVkSemaphoreNV(reinterpret_cast<GLuint64>(m_semPresent));

    submitInfo.signalSemaphoreCount = 0;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_semPresent;
    vkQueueSubmit(m_vkQueue, 1, &submitInfo, VK_NULL_HANDLE);
}
