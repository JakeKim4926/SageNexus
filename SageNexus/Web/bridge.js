/**
 * SageNexus Bridge Client
 * Native(C++) <-> WebView2(JS) JSON 메시지 규약을 구현한다.
 *
 * 규약:
 *  command  : UI -> Native (requestId 필수)
 *  response : Native -> UI (requestId 대응)
 *  event    : Native -> UI (단방향 알림)
 */
const bridgeClient = (function () {
    let _requestCounter = 0;
    const _pending = {}; // requestId -> { resolve, reject, timeoutId }

    function _generateRequestId() {
        _requestCounter += 1;
        return 'req_' + _requestCounter + '_' + Date.now();
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
            const requestId = _generateRequestId();
            const message = JSON.stringify({
                type: 'command',
                requestId: requestId,
                target: target,
                action: action,
                payload: payload || {}
            });

            const timeoutId = setTimeout(function () {
                if (_pending[requestId]) {
                    delete _pending[requestId];
                    reject(new Error('Bridge timeout: ' + target + '::' + action + ' (' + requestId + ')'));
                }
            }, 30000);

            _pending[requestId] = { resolve: resolve, reject: reject, timeoutId: timeoutId };
            window.chrome.webview.postMessage(message);
        });
    }

    function _handleResponse(message) {
        const entry = _pending[message.requestId];
        if (!entry) return;

        clearTimeout(entry.timeoutId);
        delete _pending[message.requestId];

        if (message.success) {
            entry.resolve(message.payload || {});
        } else {
            const err = message.error || {};
            entry.reject(new Error('[' + (err.code || 'ERR') + '] ' + (err.message || 'Unknown error')));
        }
    }

    function _handleEvent(message) {
        window.dispatchEvent(new CustomEvent('bridge:' + message.name, {
            detail: message.payload || {}
        }));
    }

    // Native -> UI 메시지 수신
    window.chrome.webview.addEventListener('message', function (e) {
        let message;
        try {
            message = typeof e.data === 'string' ? JSON.parse(e.data) : e.data;
        } catch (err) {
            console.error('[Bridge] Message parse error:', err, e.data);
            return;
        }

        if (message.type === 'response') {
            _handleResponse(message);
        } else if (message.type === 'event') {
            _handleEvent(message);
        }
    });

    return {
        sendCommand: sendCommand
    };
})();
