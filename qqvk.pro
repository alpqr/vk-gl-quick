TEMPLATE = app
QT += quick

SOURCES = main.cpp \
          vulkanrenderer.cpp \
          vulkanglrenderer.cpp \
          vulkanwindowrenderer.cpp

HEADERS = vulkanrenderer.h \
          vulkanglrenderer.h \
          vulkanwindowrenderer.h

RESOURCES = qqvk.qrc
OTHER_FILES = main.qml

INCLUDEPATH += $$VULKAN_INCLUDE_PATH
