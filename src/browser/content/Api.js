/*
 * Copyright (c) 2026 JVR2022
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

(function () {
    'use strict';

    var callbacks = Object.create(null);
    var nextRequestId = 0;

    var updateKeys = function (callback) {
        var requestId = String(++nextRequestId);
        if (typeof callback === 'function') {
            callbacks[requestId] = callback;
        }

        document.dispatchEvent(new CustomEvent(
            'seb-security-request',
            { detail: requestId }
        ));
    };

    document.addEventListener('seb-security-response', function (event) {
        var response;
        try {
            response = JSON.parse(String(event.detail || '{}'));
        } catch (error) {
            return;
        }

        if (!response || !response.id) {
            return;
        }

        window.SafeExamBrowser.security.browserExamKey =
            response.browserExamKey || '';
        window.SafeExamBrowser.security.configKey =
            response.configKey || '';

        var callback = callbacks[response.id];
        delete callbacks[response.id];
        if (typeof callback === 'function') {
            callback();
        }
    });

    window.SafeExamBrowser = {
        version: window.__sebVersion || 'SEB_Linux',
        security: {
            browserExamKey: '',
            configKey: '',
            updateKeys: updateKeys
        }
    };

    updateKeys();
    document.addEventListener('seb-security-bridge-ready', updateKeys);
    window.addEventListener('popstate', updateKeys);

    try {
        delete window.__sebVersion;
    } catch (error) {
        window.__sebVersion = undefined;
    }
}());
