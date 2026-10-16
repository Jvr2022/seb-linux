/*
 * Copyright (c) 2026 JVR2022
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

(function () {
    'use strict';

    if (typeof QWebChannel !== 'function' ||
        !window.qt ||
        !qt.webChannelTransport) {
        return;
    }

    new QWebChannel(qt.webChannelTransport, function (channel) {
        var bridge = channel.objects.sebSecurityBridge;
        if (!bridge) {
            return;
        }

        document.addEventListener('seb-security-request', function (event) {
            var requestId = String(event.detail || '');

            bridge.keysForUrl(window.location.href, function (keys) {
                var response = {
                    id: requestId,
                    browserExamKey: keys.browserExamKey || '',
                    configKey: keys.configKey || ''
                };

                document.dispatchEvent(new CustomEvent(
                    'seb-security-response',
                    { detail: JSON.stringify(response) }
                ));
            });
        });

        document.dispatchEvent(new CustomEvent('seb-security-bridge-ready'));
    });
}());
