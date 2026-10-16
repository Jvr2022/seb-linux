/*
 * Copyright (c) 2026 JVR2022
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

(function () {
    'use strict';

    if (typeof QWebChannel !== 'function' || !window.qt || !qt.webChannelTransport) {
        return;
    }

    new QWebChannel(qt.webChannelTransport, function (channel) {
        var bridge = channel.objects.sebSecurityApi;
        if (!bridge) {
            return;
        }

        var updateKeys = function (callback) {
            bridge.keysForUrl(window.location.href, function (keys) {
                window.SafeExamBrowser.security.browserExamKey = keys.browserExamKey || '';
                window.SafeExamBrowser.security.configKey = keys.configKey || '';
                if (typeof callback === 'function') {
                    callback();
                }
            });
        };

        window.SafeExamBrowser = {
            version: bridge.version,
            security: {
                browserExamKey: '',
                configKey: '',
                updateKeys: updateKeys
            }
        };

        updateKeys();
        window.addEventListener('popstate', updateKeys);
    });
}());
