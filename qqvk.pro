TEMPLATE = app
QT += quick

SOURCES = main.cpp \
          vulkanrenderer.cpp \
          vulkanglrenderer.cpp

HEADERS = vulkanrenderer.h \
          vulkanglrenderer.h

RESOURCES = qqvk.qrc
OTHER_FILES = main.qml

INCLUDEPATH += $$VULKAN_INCLUDE_PATH
