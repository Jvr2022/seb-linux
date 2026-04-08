QT += core gui widgets network xml

# Safe Exam Browser for Linux: Browser Engine Detection
# We support Qt WebEngine (primary) and WebKitGTK (fallback for RISC-V/Older systems).
force_qtwebengine:force_webkitgtk {
    error("CONFIG+=force_qtwebengine and CONFIG+=force_webkitgtk cannot be used together.")
}

qtwebengine_available = false
equals(QT_MAJOR_VERSION, 6):qtHaveModule(webenginecore):qtHaveModule(webenginewidgets) {
    qtwebengine_available = true
}

webkitgtk_available = false
force_webkitgtk {
    CONFIG += link_pkgconfig
    packagesExist(webkit2gtk-4.1 gtk+-3.0) {
        webkitgtk_available = true
    }
} else:!equals(qtwebengine_available, true) {
    CONFIG += link_pkgconfig
    packagesExist(webkit2gtk-4.1 gtk+-3.0) {
        webkitgtk_available = true
    }
}

seb_has_qtwebengine = 0
seb_has_webkitgtk = 0
seb_has_any_engine = 0

force_qtwebengine {
    equals(qtwebengine_available, false) {
        error("CONFIG+=force_qtwebengine was requested, but Qt WebEngine modules are unavailable.")
    }

    QT += webenginecore webenginewidgets
    seb_has_qtwebengine = 1
    seb_has_any_engine = 1
    message("Building with Qt WebEngine (forced via CONFIG+=force_qtwebengine).")
} else:force_webkitgtk {
    equals(webkitgtk_available, false) {
        error("CONFIG+=force_webkitgtk was requested, but pkg-config could not find webkit2gtk-4.1 and gtk+-3.0.")
    }

    PKGCONFIG += webkit2gtk-4.1 gtk+-3.0
    seb_has_webkitgtk = 1
    seb_has_any_engine = 1
    message("Building with WebKitGTK fallback (forced via CONFIG+=force_webkitgtk).")
} else:equals(qtwebengine_available, true) {
    QT += webenginecore webenginewidgets
    seb_has_qtwebengine = 1
    seb_has_any_engine = 1
    message("Building with Qt WebEngine (auto-detected).")
} else:equals(webkitgtk_available, true) {
    PKGCONFIG += webkit2gtk-4.1 gtk+-3.0
    seb_has_webkitgtk = 1
    seb_has_any_engine = 1
    message("Building with WebKitGTK fallback (Qt WebEngine unavailable; auto-detected).")
} else {
    warning("No supported browser engine found (QtWebEngine/WebKitGTK missing). The app will show an error on startup.")
}

DEFINES += SEB_HAS_QTWEBENGINE=$$seb_has_qtwebengine
DEFINES += SEB_HAS_WEBKITGTK=$$seb_has_webkitgtk
DEFINES += SEB_HAS_ANY_ENGINE=$$seb_has_any_engine

# Dev Bypass Build Option
# Usage: qmake CONFIG+=dev_bypass
dev_bypass {
    DEFINES += SEB_DEV_BYPASS_DEFAULT=1
    message("Building with PERSISTENT developer bypass enabled.")
}

CONFIG += c++20 console warn_on object_parallel_to_source
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
    src/applications/application_factory.cpp \
    src/applications/application_manager.cpp \
    src/applications/application_window.cpp \
    src/applications/external_application.cpp \
    src/applications/external_application_instance.cpp \
    src/client/client_controller.cpp \
    src/client/composition_root.cpp \
    src/client/coordinator.cpp \
    src/client/notifications/about_notification.cpp \
    src/client/notifications/log_notification.cpp \
    src/client/notifications/verificator_notification.cpp \
    src/client/operations/application_operation.cpp \
    src/client/operations/browser_operation.cpp \
    src/client/operations/client_operation.cpp \
    src/client/operations/client_operation_sequence.cpp \
    src/client/operations/shell_operation.cpp \
    src/client/responsibilities/browser_responsibility.cpp \
    src/client/responsibilities/client_responsibility.cpp \
    src/client/responsibilities/shell_responsibility.cpp \
    src/communication/hosts/base_host.cpp \
    src/communication/hosts/host_object_factory.cpp \
    src/communication/proxies/base_proxy.cpp \
    src/communication/proxies/client_proxy.cpp \
    src/communication/proxies/proxy_factory.cpp \
    src/communication/proxies/proxy_object_factory.cpp \
    src/communication/proxies/runtime_proxy.cpp \
    src/communication/proxies/service_proxy.cpp \
    src/configuration/configuration_data/data_mapper.cpp \
    src/configuration/configuration_data/data_processor.cpp \
    src/configuration/configuration_data/data_values.cpp \
    src/configuration/configuration_data/data_mapping/application_data_mapper.cpp \
    src/configuration/configuration_data/data_mapping/audio_data_mapper.cpp \
    src/configuration/configuration_data/data_mapping/base_data_mapper.cpp \
    src/configuration/configuration_data/data_mapping/browser_data_mapper.cpp \
    src/configuration/configuration_data/data_mapping/configuration_file_data_mapper.cpp \
    src/configuration/configuration_data/data_mapping/display_data_mapper.cpp \
    src/configuration/configuration_data/data_mapping/general_data_mapper.cpp \
    src/configuration/configuration_data/data_mapping/input_data_mapper.cpp \
    src/configuration/configuration_data/data_mapping/proctoring_data_mapper.cpp \
    src/configuration/configuration_data/data_mapping/security_data_mapper.cpp \
    src/configuration/configuration_data/data_mapping/server_data_mapper.cpp \
    src/configuration/configuration_data/data_mapping/service_data_mapper.cpp \
    src/configuration/configuration_data/data_mapping/system_data_mapper.cpp \
    src/configuration/configuration_data/data_mapping/user_interface_data_mapper.cpp \
    src/configuration/configuration_data/json.cpp \
    src/configuration/configuration_repository.cpp \
    src/configuration/cryptography/certificate_store.cpp \
    src/configuration/cryptography/hash_algorithm.cpp \
    src/configuration/cryptography/key_generator.cpp \
    src/configuration/cryptography/password_encryption.cpp \
    src/configuration/cryptography/public_key_encryption.cpp \
    src/configuration/cryptography/public_key_symmetric_encryption.cpp \
    src/configuration/data_compression/gzip_compressor.cpp \
    src/configuration/data_formats/binary_block.cpp \
    src/configuration/data_formats/binary_parser.cpp \
    src/configuration/data_formats/binary_serializer.cpp \
    src/configuration/data_formats/xml_element.cpp \
    src/configuration/data_formats/xml_parser.cpp \
    src/configuration/data_formats/xml_serializer.cpp \
    src/configuration/data_resources/file_resource_loader.cpp \
    src/configuration/data_resources/file_resource_saver.cpp \
    src/configuration/data_resources/network_resource_loader.cpp \
    src/configuration/sub_stream.cpp \
    src/browser/BrowserApplication.cpp \
    src/browser/BrowserControl.cpp \
    src/browser/BrowserWindow.cpp \
    src/browser/Clipboard.cpp \
    src/browser/engines/qtwebengine/qt_webengine_profile.cpp \
    src/browser/engines/qtwebengine/qt_webengine_provider.cpp \
    src/browser/engines/qtwebengine/qt_webengine_view.cpp \
    src/browser/engines/webkitgtk/webkitgtk_profile.cpp \
    src/browser/engines/webkitgtk/webkitgtk_provider.cpp \
    src/browser/engines/webkitgtk/webkitgtk_view.cpp \
    src/browser/key_generator.cpp \
    src/browser/request_filter.cpp \
    src/browser/request_interceptor.cpp \
    src/browser/engines/engine_factory.cpp \
    src/browser/webengine_environment.cpp \
    src/app_controller.cpp \
    src/main.cpp \
    src/settings/password_container.cpp \
    src/settings/resource_loader.cpp \
    src/settings/settings_defaults.cpp \
    src/settings/settings_mapping.cpp \
    src/settings/settings_parsing.cpp \
    src/shell/taskbar/taskbar_service.cpp \
    src/shell/taskbar/platform/audio_controller.cpp \
    src/shell/taskbar/platform/battery_controller.cpp \
    src/shell/taskbar/platform/command_helper.cpp \
    src/shell/taskbar/platform/keyboard_controller.cpp \
    src/shell/taskbar/platform/network_controller.cpp \
    src/ui/taskbar/common/taskbar_button.cpp \
    src/ui/taskbar/common/taskbar_popup.cpp \
    src/ui/taskbar/controls/application_control.cpp \
    src/ui/taskbar/controls/audio_control.cpp \
    src/ui/taskbar/controls/battery_control.cpp \
    src/ui/taskbar/controls/clock_widget.cpp \
    src/ui/taskbar/controls/external_application_control.cpp \
    src/ui/taskbar/controls/keyboard_control.cpp \
    src/ui/taskbar/controls/network_control.cpp \
    src/ui/taskbar/controls/notification_control.cpp \
    src/ui/taskbar/controls/raise_hand_control.cpp \
    src/ui/taskbar/controls/window_list_popup.cpp \
    src/ui/taskbar/taskbar_widget.cpp \
    src/ui/runtime_window.cpp \
    src/ui/seb_taskbar.cpp \
    src/seb_settings.cpp \
    src/seb_session.cpp \
    src/security/security_service.cpp \
    src/browser_window.cpp
HEADERS += \
    src/app_controller.h \
    src/applications/application_factory.h \
    src/applications/application_manager.h \
    src/applications/application_window.h \
    src/applications/contracts/application.h \
    src/applications/contracts/application_factory.h \
    src/applications/contracts/application_window.h \
    src/applications/contracts/events/icon_changed_event_handler.h \
    src/applications/contracts/events/title_changed_event_handler.h \
    src/applications/contracts/events/windows_changed_event_handler.h \
    src/applications/contracts/factory_result.h \
    src/applications/external_application.h \
    src/applications/external_application_instance.h \
    src/client/client_context.h \
    src/client/client_controller.h \
    src/client/composition_root.h \
    src/client/contracts/coordinator.h \
    src/client/coordinator.h \
    src/client/notifications/about_notification.h \
    src/client/notifications/log_notification.h \
    src/client/notifications/verificator_notification.h \
    src/client/operations/application_operation.h \
    src/client/operations/browser_operation.h \
    src/client/operations/client_operation.h \
    src/client/operations/client_operation_sequence.h \
    src/client/operations/shell_operation.h \
    src/client/responsibilities/browser_responsibility.h \
    src/client/responsibilities/client_responsibility.h \
    src/client/responsibilities/client_task.h \
    src/client/responsibilities/shell_responsibility.h \
    src/communication/contracts/data/authentication_response.h \
    src/communication/contracts/data/configuration_response.h \
    src/communication/contracts/data/connection_response.h \
    src/communication/contracts/data/disconnection_message.h \
    src/communication/contracts/data/disconnection_response.h \
    src/communication/contracts/data/exam_selection_reply_message.h \
    src/communication/contracts/data/exam_selection_request_message.h \
    src/communication/contracts/data/message.h \
    src/communication/contracts/data/message_box_reply_message.h \
    src/communication/contracts/data/message_box_request_message.h \
    src/communication/contracts/data/password_reply_message.h \
    src/communication/contracts/data/password_request_message.h \
    src/communication/contracts/data/password_request_purpose.h \
    src/communication/contracts/data/reconfiguration_denied_message.h \
    src/communication/contracts/data/reconfiguration_message.h \
    src/communication/contracts/data/response.h \
    src/communication/contracts/data/server_failure_action_reply_message.h \
    src/communication/contracts/data/server_failure_action_request_message.h \
    src/communication/contracts/data/session_start_message.h \
    src/communication/contracts/data/session_stop_message.h \
    src/communication/contracts/data/simple_message.h \
    src/communication/contracts/data/simple_message_purport.h \
    src/communication/contracts/data/simple_response.h \
    src/communication/contracts/data/simple_response_purport.h \
    src/communication/contracts/events/client_configuration_event_args.h \
    src/communication/contracts/events/communication_event_args.h \
    src/communication/contracts/events/communication_event_handler.h \
    src/communication/contracts/events/exam_selection_reply_event_args.h \
    src/communication/contracts/events/exam_selection_request_event_args.h \
    src/communication/contracts/events/message_box_reply_event_args.h \
    src/communication/contracts/events/message_box_request_event_args.h \
    src/communication/contracts/events/password_reply_event_args.h \
    src/communication/contracts/events/password_request_event_args.h \
    src/communication/contracts/events/reconfiguration_event_args.h \
    src/communication/contracts/events/server_failure_action_reply_event_args.h \
    src/communication/contracts/events/server_failure_action_request_event_args.h \
    src/communication/contracts/events/session_start_event_args.h \
    src/communication/contracts/events/session_stop_event_args.h \
    src/communication/contracts/hosts/i_client_host.h \
    src/communication/contracts/hosts/i_host_object.h \
    src/communication/contracts/hosts/i_host_object_factory.h \
    src/communication/contracts/hosts/i_runtime_host.h \
    src/communication/contracts/hosts/i_service_host.h \
    src/communication/contracts/i_communication.h \
    src/communication/contracts/i_communication_host.h \
    src/communication/contracts/i_communication_proxy.h \
    src/communication/contracts/interlocutor.h \
    src/communication/contracts/proxies/communication_result.h \
    src/communication/contracts/proxies/i_client_proxy.h \
    src/communication/contracts/proxies/i_proxy_factory.h \
    src/communication/contracts/proxies/i_proxy_object.h \
    src/communication/contracts/proxies/i_proxy_object_factory.h \
    src/communication/contracts/proxies/i_runtime_proxy.h \
    src/communication/contracts/proxies/i_service_proxy.h \
    src/communication/hosts/base_host.h \
    src/communication/hosts/host_object_factory.h \
    src/communication/proxies/base_proxy.h \
    src/communication/proxies/client_proxy.h \
    src/communication/proxies/proxy_factory.h \
    src/communication/proxies/proxy_object_factory.h \
    src/communication/proxies/runtime_proxy.h \
    src/communication/proxies/service_proxy.h \
    src/configuration/Properties/AssemblyInfo.h \
    src/configuration/configuration_data/data_mapper.h \
    src/configuration/configuration_data/data_processor.h \
    src/configuration/configuration_data/data_values.h \
    src/configuration/configuration_data/data_mapping/application_data_mapper.h \
    src/configuration/configuration_data/data_mapping/audio_data_mapper.h \
    src/configuration/configuration_data/data_mapping/base_data_mapper.h \
    src/configuration/configuration_data/data_mapping/browser_data_mapper.h \
    src/configuration/configuration_data/data_mapping/configuration_file_data_mapper.h \
    src/configuration/configuration_data/data_mapping/display_data_mapper.h \
    src/configuration/configuration_data/data_mapping/general_data_mapper.h \
    src/configuration/configuration_data/data_mapping/input_data_mapper.h \
    src/configuration/configuration_data/data_mapping/proctoring_data_mapper.h \
    src/configuration/configuration_data/data_mapping/security_data_mapper.h \
    src/configuration/configuration_data/data_mapping/server_data_mapper.h \
    src/configuration/configuration_data/data_mapping/service_data_mapper.h \
    src/configuration/configuration_data/data_mapping/system_data_mapper.h \
    src/configuration/configuration_data/data_mapping/user_interface_data_mapper.h \
    src/configuration/configuration_data/json.h \
    src/configuration/configuration_data/keys.h \
    src/configuration/configuration_repository.h \
    src/configuration/contracts/Properties/AssemblyInfo.h \
    src/configuration/contracts/app_config.h \
    src/configuration/contracts/client_configuration.h \
    src/configuration/contracts/cryptography/encryption_parameters.h \
    src/configuration/contracts/cryptography/i_certificate_store.h \
    src/configuration/contracts/cryptography/i_hash_algorithm.h \
    src/configuration/contracts/cryptography/i_key_generator.h \
    src/configuration/contracts/cryptography/i_password_encryption.h \
    src/configuration/contracts/cryptography/i_public_key_encryption.h \
    src/configuration/contracts/cryptography/password_parameters.h \
    src/configuration/contracts/cryptography/public_key_parameters.h \
    src/configuration/contracts/data_compression/i_data_compressor.h \
    src/configuration/contracts/data_formats/format_type.h \
    src/configuration/contracts/data_formats/i_data_parser.h \
    src/configuration/contracts/data_formats/i_data_serializer.h \
    src/configuration/contracts/data_formats/parse_result.h \
    src/configuration/contracts/data_formats/serialize_result.h \
    src/configuration/contracts/data_resources/i_resource_loader.h \
    src/configuration/contracts/data_resources/i_resource_saver.h \
    src/configuration/contracts/i_configuration_repository.h \
    src/configuration/contracts/load_status.h \
    src/configuration/contracts/save_status.h \
    src/configuration/contracts/service_configuration.h \
    src/configuration/contracts/session_configuration.h \
    src/configuration/cryptography/certificate_store.h \
    src/configuration/cryptography/hash_algorithm.h \
    src/configuration/cryptography/key_generator.h \
    src/configuration/cryptography/password_encryption.h \
    src/configuration/cryptography/public_key_encryption.h \
    src/configuration/cryptography/public_key_symmetric_encryption.h \
    src/configuration/data_compression/gzip_compressor.h \
    src/configuration/data_formats/binary_block.h \
    src/configuration/data_formats/binary_parser.h \
    src/configuration/data_formats/binary_serializer.h \
    src/configuration/data_formats/xml_element.h \
    src/configuration/data_formats/xml_parser.h \
    src/configuration/data_formats/xml_serializer.h \
    src/configuration/data_resources/file_resource_loader.h \
    src/configuration/data_resources/file_resource_saver.h \
    src/configuration/data_resources/network_resource_loader.h \
    src/configuration/sub_stream.h \
    src/browser/BrowserApplication.h \
    src/browser/BrowserApplicationContext.h \
    src/browser/BrowserControl.h \
    src/browser/BrowserWindow.h \
    src/browser/BrowserWindowContext.h \
    src/browser/Clipboard.h \
    src/browser/contracts/IBrowserApplication.h \
    src/browser/contracts/IBrowserWindow.h \
    src/browser/contracts/i_engine_provider.h \
    src/browser/contracts/i_webprofile.h \
    src/browser/contracts/i_webview.h \
    src/browser/engines/qtwebengine/qt_webengine_profile.h \
    src/browser/engines/qtwebengine/qt_webengine_provider.h \
    src/browser/engines/qtwebengine/qt_webengine_view.h \
    src/browser/engines/webkitgtk/webkitgtk_profile.h \
    src/browser/engines/webkitgtk/webkitgtk_provider.h \
    src/browser/engines/webkitgtk/webkitgtk_view.h \
    src/browser/key_generator.h \
    src/browser/request_filter.h \
    src/browser/request_interceptor.h \
    src/browser/engines/engine_factory.h \
    src/browser/webengine_environment.h \
    src/settings/password_container.h \
    src/settings/resource_loader.h \
    src/settings/settings_defaults.h \
    src/settings/settings_mapping.h \
    src/settings/settings_parsing.h \
    src/shell/taskbar/taskbar_model.h \
    src/shell/taskbar/platform/audio_controller.h \
    src/shell/taskbar/platform/battery_controller.h \
    src/shell/taskbar/platform/command_helper.h \
    src/shell/taskbar/platform/keyboard_controller.h \
    src/shell/taskbar/platform/network_controller.h \
    src/shell/taskbar/taskbar_service.h \
    src/ui/taskbar/common/taskbar_button.h \
    src/ui/taskbar/common/taskbar_popup.h \
    src/ui/taskbar/common/taskbar_style.h \
    src/ui/taskbar/controls/application_control.h \
    src/ui/taskbar/controls/audio_control.h \
    src/ui/taskbar/controls/battery_control.h \
    src/ui/taskbar/controls/clock_widget.h \
    src/ui/taskbar/controls/external_application_control.h \
    src/ui/taskbar/controls/keyboard_control.h \
    src/ui/taskbar/controls/network_control.h \
    src/ui/taskbar/controls/notification_control.h \
    src/ui/taskbar/controls/raise_hand_control.h \
    src/ui/taskbar/controls/window_list_popup.h \
    src/ui/taskbar/taskbar_widget.h \
    src/ui/runtime_window.h \
    src/ui/seb_taskbar.h \
    src/seb_settings.h \
    src/seb_session.h \
    src/security/security_service.h \
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
