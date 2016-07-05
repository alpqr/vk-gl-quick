Qt-based Vulkan experiments for QWindow and GL_NV_draw_vulkan_image in Qt Quick
===============================================================================

When building against a Vulkan SDK where the headers are not available in the default search paths,
run 'qmake VULKAN_INCLUDE_PATH=c:/VulkanSDK/1.0.17.0/include/vulkan' to specify the location of vulkan.h

There are two modes:

(1)
By default the application opens an OpenGL-based Qt Quick window and connects
to QQuickWindow::beforeRendering() to perform some rendering via Vulkan before
adding the rest of the Quick scene as a GL "overlay".

Currently the rendering is merely clearing to green. Qt then fades out a red rectangle on top.

This needs GL_NV_draw_vulkan_image, and has been tested on Windows (NVIDIA 365.xx)
and the DRIVE CX embedded platform (NVIDIA 367.xx, OpenGL ES 3.2).

(2) Passing --window drops the Qt Quick and OpenGL path, and opens a plain
QWindow instead. Currently only the Win32 WSI is supported. This mode can be a
lot easier to develop with since it allows using the validation layers.

Initialization and teardown is currently tied to
scenegraphInitialized/Invalidated and expose/obscure events. When these happen
is windowing system dependent. Releasing all resources on obscure is not
necessarily ideal for real apps but will do for demo purposes.

To be as portable as possible, all functions are resolved dynamically, either
via QLibrary or the device/instance-level getProcAddr, so no libs are needed at
link time.

-TODO:
-- add the swap chain stuff for the window path
-- issue some real draw calls
-- play a bit with shaders (SPIR-V, VK_NV_glsl_shader, etc.)
-- some other WSIs
-- long-term: investigate doing things in a QQuickFramebufferObject to get an item that is actually part of the scene
