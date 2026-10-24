import QtQuick 2.2
import Sailfish.Silica 1.0
import Sailfish.WebView 1.0
import harbour.hydroxide 1.0

Page {
    id: captchaPage
    objectName: "captchaPage"
    allowedOrientations: Orientation.All

    // URL to the local hydroxide challenge proxy server (e.g. http://127.0.0.1:<port>/)
    property string captchaUrl: ""
    property BridgeHelper bridge: null

    PageHeader {
        id: header
        title: "Proton Verification"
        description: captchaUrl.length > 0 ? captchaUrl : "127.0.0.1"
    }

    ProgressBar {
        id: progressBar
        anchors.top: header.bottom
        width: parent.width
        minimumValue: 0
        maximumValue: 100
        value: webView.loadProgress
        visible: webView.loading
        z: 2
    }

    WebView {
        id: webView
        anchors.top: progressBar.visible ? progressBar.bottom : header.bottom
        anchors.bottom: bottomBar.top
        anchors.left: parent.left
        anchors.right: parent.right
        url: (captchaPage.captchaUrl.indexOf("://") !== -1) ? captchaPage.captchaUrl : ("http://" + captchaPage.captchaUrl)
        active: Qt.application.active

        onLoadedChanged: {
            if (loaded) {
                // When the user completes the CAPTCHA, hydroxide's injected script displays
                // "Verification complete, you can close this tab and go back to hydroxide."
                // and sends the token via POST /hydroxide/token.
                runJavaScript("document.body ? document.body.innerText : ''", function(result) {
                    if (result && result.indexOf("Verification complete") !== -1) {
                        if (bridge) {
                            bridge.onCaptchaCompleted();
                        }
                    }
                });
            }
        }
    }

    Item {
        id: bottomBar
        anchors.bottom: parent.bottom
        width: parent.width
        height: Theme.itemSizeMedium

        Row {
            anchors.centerIn: parent
            spacing: Theme.paddingLarge

            Button {
                text: "Reload"
                onClicked: {
                    if (webView) {
                        webView.reload();
                    }
                }
            }

            Button {
                text: "Cancel"
                onClicked: {
                    if (bridge) {
                        bridge.cancelAuth();
                    }
                    pageStack.pop();
                }
            }
        }
    }

    Connections {
        target: bridge
        onBridgePasswordChanged: {
            // Once Hydroxide receives the token and completes auth,
            // the bridge password is set. Immediately return to MainPage!
            if (bridge && bridge.bridgePassword.length > 0) {
                pageStack.pop();
            }
        }
        onCaptchaRequiredChanged: {
            if (bridge && !bridge.captchaRequired && bridge.bridgePassword.length > 0) {
                pageStack.pop();
            }
        }
        onLastErrorChanged: {
            if (bridge && bridge.lastError.length > 0) {
                pageStack.pop();
            }
        }
    }

    Component.onCompleted: {
        if (bridge && bridge.bridgePassword.length > 0) {
            pageStack.pop();
        }
    }
}
