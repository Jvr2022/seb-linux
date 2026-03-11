QT += core gui widgets network xml webenginecore webenginewidgets

CONFIG += c++20 console warn_on
TEMPLATE = app
TARGET = safe-exam-browser

QMAKE_CXXFLAGS += -Wall -Wextra -Wpedantic

INCLUDEPATH += src
LIBS += -lz -lcrypto

OBJECTS_DIR = $$OUT_PWD/.obj
MOC_DIR = $$OUT_PWD/.moc
RCC_DIR = $$OUT_PWD/.rcc
UI_DIR = $$OUT_PWD/.ui
DESTDIR = $$OUT_PWD/bin

SOURCES += \
    src/browser/key_generator.cpp \
    src/browser/request_filter.cpp \
    src/browser/request_interceptor.cpp \
    src/browser/webengine_environment.cpp \
    src/app_controller.cpp \
    src/main.cpp \
    src/settings/password_container.cpp \
    src/settings/resource_loader.cpp \
    src/settings/settings_defaults.cpp \
    src/settings/settings_mapping.cpp \
    src/settings/settings_parsing.cpp \
    src/ui/runtime_window.cpp \
    src/ui/seb_taskbar.cpp \
    src/seb_settings.cpp \
    src/seb_session.cpp \
    src/browser_window.cpp

HEADERS += \
    src/app_controller.h \
    src/browser/key_generator.h \
    src/browser/request_filter.h \
    src/browser/request_interceptor.h \
    src/browser/webengine_environment.h \
    src/settings/password_container.h \
    src/settings/resource_loader.h \
    src/settings/settings_defaults.h \
    src/settings/settings_mapping.h \
    src/settings/settings_parsing.h \
    src/ui/runtime_window.h \
    src/ui/seb_taskbar.h \
    src/seb_settings.h \
    src/seb_session.h \
    src/browser_window.h

RESOURCES += resources.qrc

target.path = /usr/bin
desktop.files = packaging/linux/safe-exam-browser.desktop
desktop.path = /usr/share/applications
mime.files = packaging/linux/safe-exam-browser.xml
mime.path = /usr/share/mime/packages
metainfo.files = packaging/linux/safe-exam-browser.metainfo.xml
metainfo.path = /usr/share/metainfo
icon.files = assets/icons/safe-exam-browser.png
icon.path = /usr/share/icons/hicolor/256x256/apps
icon64.files = assets/icons/64x64/safe-exam-browser.png
icon64.path = /usr/share/icons/hicolor/64x64/apps
examples.files = examples/minimal-config.json
examples.path = /usr/share/doc/safe-exam-browser/examples

INSTALLS += target desktop mime metainfo icon icon64 examples
