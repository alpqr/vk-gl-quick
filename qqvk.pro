TEMPLATE = app
QT += quick

SOURCES = main.cpp \
          rendervk.cpp

HEADERS = rendervk.h

RESOURCES = qqvk.qrc
OTHER_FILES = main.qml

INCLUDEPATH += $$VULKAN_INCLUDE_PATH
