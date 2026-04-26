/**
 * SageNexus Bridge Client
 * Native(C++) <-> WebView2(JS) JSON 메시지 규약을 구현한다.
 *
 * 규약:
 *  command  : UI -> Native (requestId 필수)
 *  response : Native -> UI (requestId 대응)
 *  event    : Native -> UI (단방향 알림)
 *
 * iframe 지원:
 *  플러그인 페이지는 iframe 안에서 로드된다. WebView2에서 iframe의
 *  window.chrome.webview.postMessage는 메인 프레임의 WebMessageReceived에 도달하지 않는다.
 *  따라서 iframe에서는 window.parent.postMessage로 메인 프레임에 relay하고,
 *  메인 프레임이 window.chrome.webview.postMessage로 네이티브에 전달한다.
 */
window.bridgeClient = (function () {
    var _requestCounter = 0;
    var _pending = {}; // requestId -> { resolve, reject, timeoutId }

    function _generateRequestId() {
        _requestCounter += 1;
        return 'req_' + _requestCounter + '_' + Date.now();
    }

    function _isInIframe() {
        try { return window !== window.top; } catch (e) { return true; }
    }

    function _postToNative(message) {
        if (_isInIframe()) {
            // iframe 컨텍스트: 메인 프레임에 relay 요청
            window.parent.postMessage({ __bridgeRelay: true, data: message }, '*');
        } else {
            window.chrome.webview.postMessage(message);
        }
    }

    /**
     * Native에 command를 전송하고 Promise를 반환한다.
     * @param {string} target  - 대상 (e.g. "navigation", "data")
     * @param {string} action  - 액션 (e.g. "navigate", "load")
     * @param {object} payload - 추가 데이터
     * @returns {Promise<object>} response.payload
     */
    function sendCommand(target, action, payload) {
        return new Promise(function (resolve, reject) {
            var requestId = _generateRequestId();
            var message = JSON.stringify({
                type: 'command',
                requestId: requestId,
                target: target,
                action: action,
                payload: payload || {}
            });

            var timeoutId = setTimeout(function () {
                if (_pending[requestId]) {
                    delete _pending[requestId];
                    reject(new Error('Bridge timeout: ' + target + '::' + action + ' (' + requestId + ')'));
                }
            }, 30000);

            _pending[requestId] = { resolve: resolve, reject: reject, timeoutId: timeoutId };
            _postToNative(message);
        });
    }

    function _handleResponse(message) {
        var entry = _pending[message.requestId];
        if (!entry) return;

        clearTimeout(entry.timeoutId);
        delete _pending[message.requestId];

        if (message.success) {
            entry.resolve(message.payload || {});
        } else {
            var err = message.error || {};
            entry.reject(new Error('[' + (err.code || 'ERR') + '] ' + (err.message || 'Unknown error')));
        }
    }

    function _handleEvent(message) {
        window.dispatchEvent(new CustomEvent('bridge:' + message.name, {
            detail: message.payload || {}
        }));
    }

    function _dispatch(message) {
        if (!message || typeof message !== 'object') return;
        if (message.type === 'response') {
            _handleResponse(message);
        } else if (message.type === 'event') {
            _handleEvent(message);
        }
    }

    // Native -> UI: ExecuteScript 기반 진입점.
    // 네이티브는 window.__bridgeReceive({...}) 형태로 JSON 리터럴을 직접 전달한다.
    window.__bridgeReceive = function (message) {
        try {
            _dispatch(typeof message === 'string' ? JSON.parse(message) : message);
        } catch (err) {
            console.error('[Bridge] __bridgeReceive error:', err);
        }
    };

    if (!_isInIframe()) {
        // 메인 프레임 전용: 레거시 PostWebMessageAsString 수신
        window.chrome.webview.addEventListener('message', function (e) {
            var message;
            try {
                message = typeof e.data === 'string' ? JSON.parse(e.data) : e.data;
            } catch (err) {
                console.error('[Bridge] Message parse error:', err, e.data);
                return;
            }
            _dispatch(message);
        });

        // 메인 프레임 전용: iframe relay 수신 후 네이티브로 전달
        window.addEventListener('message', function (e) {
            if (e.data && e.data.__bridgeRelay) {
                window.chrome.webview.postMessage(e.data.data);
            }
        });
    }

    return {
        sendCommand: sendCommand
    };
})();
