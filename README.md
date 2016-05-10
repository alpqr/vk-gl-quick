Experiments with GL_NV_draw_vulkan_image in Qt Quick apps
=========================================================

Connect to QQuickWindow::beforeRendering() to perform some rendering via Vulkan before adding
the rest of the Quick scene as GL "overlay".

Currently the rendering is merely clearing to green. Qt then fades out a red rectangle on top.

TODO:
- test on a real embedded board (this is currently our best shot on certain systems due to the lack of a WSI)
- issue some real draw calls
- try out VK_NV_glsl_shader
- long-term: investigate doing things in a QQuickFramebufferObject to get an item that is actually part of the scene
