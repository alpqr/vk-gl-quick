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

#include "rendervk.h"
#include <QQuickWindow>
#include <QOpenGLContext>

VulkanRenderer::VulkanRenderer(QQuickWindow *window)
    : m_window(window)
{
    connect(window, &QQuickWindow::beforeRendering, this, &VulkanRenderer::render, Qt::DirectConnection);
    connect(window, &QQuickWindow::sceneGraphInvalidated, this, &VulkanRenderer::cleanup, Qt::DirectConnection);
    connect(window, &QQuickWindow::sceneGraphAboutToStop, this, &VulkanRenderer::cleanup, Qt::DirectConnection);
}

void VulkanRenderer::resize()
{
    if (!m_inited || m_window->size().isEmpty())
        return;

    vkDeviceWaitIdle(m_vkDev);

    qDebug("resize %dx%d", m_window->width(), m_window->height());
    releaseRenderTarget();
    createRenderTarget(m_window->size());
}

void VulkanRenderer::init()
{
    if (m_inited)
        return;

    QOpenGLContext *ctx = QOpenGLContext::currentContext();

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

    VkApplicationInfo appInfo;
    memset(&appInfo, 0, sizeof(appInfo));
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "qqvk";
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 2);

    VkInstanceCreateInfo instInfo;
    memset(&instInfo, 0, sizeof(instInfo));
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pApplicationInfo = &appInfo;

    VkResult err = vkCreateInstance(&instInfo, nullptr, &m_vkInst);
    if (err != VK_SUCCESS)
        qFatal("Failed to create Vulkan instance: %d", err);

    // Just pick the first physical device.
    uint32_t devCount = 1;
    err = vkEnumeratePhysicalDevices(m_vkInst, &devCount, &m_vkPhysDev);
    if (err != VK_SUCCESS)
        qFatal("Failed to enumerate physical devices: %d", err);

    qDebug("%d physical devices", devCount);

    VkPhysicalDeviceProperties physDevProps;
    vkGetPhysicalDeviceProperties(m_vkPhysDev, &physDevProps);
    qDebug("Device name: %s\nDriver version: %d.%d.%d", physDevProps.deviceName,
           VK_VERSION_MAJOR(physDevProps.driverVersion), VK_VERSION_MINOR(physDevProps.driverVersion),
           VK_VERSION_PATCH(physDevProps.driverVersion));

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

    err = vkCreateDevice(m_vkPhysDev, &devInfo, nullptr, &m_vkDev);
    if (err != VK_SUCCESS)
        qFatal("Failed to create device: %d", err);

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

    VkSemaphoreCreateInfo semInfo;
    memset(&semInfo, 0, sizeof(semInfo));
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    err = vkCreateSemaphore(m_vkDev, &semInfo, nullptr, &m_semRender);
    if (err != VK_SUCCESS)
        qFatal("Failed to create render semaphore");

    memset(&semInfo, 0, sizeof(semInfo));
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_vkDev, &semInfo, nullptr, &m_semPresent);
    if (err != VK_SUCCESS)
        qFatal("Failed to create present semaphore");

    createRenderTarget(m_window->size());

    m_inited = true;
    qDebug("VK-on-GL renderer initialized");
}

void VulkanRenderer::cleanup()
{
    if (!m_inited)
        return;

    qDebug("Stopping VK-on-GL renderer");
    m_inited = false;

    releaseRenderTarget();

    vkDestroySemaphore(m_vkDev, m_semRender, nullptr);
    vkDestroySemaphore(m_vkDev, m_semPresent, nullptr);
    vkDestroyCommandPool(m_vkDev, m_vkCmdPool, nullptr);
    vkDestroyDevice(m_vkDev, nullptr);
    vkDestroyInstance(m_vkInst, nullptr);
}

static inline VkDeviceSize alignedSize(VkDeviceSize size, VkDeviceSize byteAlign)
{
    return (size + byteAlign - 1) & ~(byteAlign - 1);
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
    unsigned long memTypeIndex = 0;
    if (colorMemReq.memoryTypeBits) {
#if defined(Q_CC_GNU)
        memTypeIndex = __builtin_ctz(colorMemReq.memoryTypeBits);
#elif defined(Q_CC_MSVC)
        _BitScanForward(&memTypeIndex, colorMemReq.memoryTypeBits);
#endif
    }

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

void VulkanRenderer::render()
{
    init();

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
    rpBeginInfo.renderArea.extent.width = m_window->width();
    rpBeginInfo.renderArea.extent.height = m_window->height();
    rpBeginInfo.clearValueCount = 1;
    VkClearColorValue clearColor = { 0.0f, 1.0f, 0.0f, 1.0f };
    VkClearValue clearValue;
    clearValue.color = clearColor;
    clearValue.depthStencil.depth = 1.0f;
    clearValue.depthStencil.stencil = 0;
    rpBeginInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = { 0, 0, float(m_window->width()), float(m_window->height()), 0, 1 };
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

    present();
}

void VulkanRenderer::present()
{
    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(submitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_semRender;
    vkQueueSubmit(m_vkQueue, 1, &submitInfo, VK_NULL_HANDLE);

    glWaitVkSemaphoreNV(reinterpret_cast<GLuint64>(m_semRender));
    glDrawVkImageNV(reinterpret_cast<GLuint64>(m_color), 0,
                    0, 0, m_window->width(), m_window->height(), 0,
                    0, 1, 1, 0);
    glSignalVkSemaphoreNV(reinterpret_cast<GLuint64>(m_semPresent));

    submitInfo.signalSemaphoreCount = 0;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_semPresent;
    vkQueueSubmit(m_vkQueue, 1, &submitInfo, VK_NULL_HANDLE);
}
