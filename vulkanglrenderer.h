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

#ifndef VULKANGLRENDERER_H
#define VULKANGLRENDERER_H

#include "vulkanrenderer.h"
#include <QObject>
#include <qopengl.h>

class QQuickWindow;

class VulkanGLRenderer : public QObject, public VulkanRenderer
{
public:
    VulkanGLRenderer(QQuickWindow *window);

private slots:
    void onBeforeGLRendering();
    void onInvalidate();

private:
    void init();
    void present();

    QQuickWindow *m_quickWindow;
    bool m_inited = false;
    QSize m_lastWindowSize;
    VkSemaphore m_semRender;
    VkSemaphore m_semPresent;

    typedef PFN_vkVoidFunction (QOPENGLF_APIENTRY * PFN_glGetVkProcAddrNV) (const GLchar *name);
    typedef void (QOPENGLF_APIENTRY * PFN_glWaitVkSemaphoreNV) (GLuint64 vkSemaphore);
    typedef void (QOPENGLF_APIENTRY * PFN_glSignalVkSemaphoreNV) (GLuint64 vkSemaphore);
    typedef void (QOPENGLF_APIENTRY * PFN_glSignalVkFenceNV) (GLuint64 vkFence);
    typedef void (QOPENGLF_APIENTRY * PFN_glDrawVkImageNV) (GLuint64 vkImage, GLuint sampler,
                                                       GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLfloat z,
                                                       GLfloat s0, GLfloat t0, GLfloat s1, GLfloat t1);

    PFN_glGetVkProcAddrNV glGetVkProcAddrNV;
    PFN_glWaitVkSemaphoreNV glWaitVkSemaphoreNV;
    PFN_glSignalVkSemaphoreNV glSignalVkSemaphoreNV;
    PFN_glSignalVkFenceNV glSignalVkFenceNV;
    PFN_glDrawVkImageNV glDrawVkImageNV;
};

#endif
