import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.hydroxide 1.0

Page {
    id: page
    objectName: "mainPage"
    allowedOrientations: Orientation.All

    property bool revealPassword: false

    BridgeHelper {
        id: bridge

        function navigateToCaptcha(targetUrl) {
            var url = targetUrl ? targetUrl : bridge.captchaUrl;
            if (url && url.length > 0) {
                if (pageStack.currentPage && pageStack.currentPage.objectName === "captchaPage") {
                    return;
                }
                pageStack.push(Qt.resolvedUrl("CaptchaPage.qml"), {
                    captchaUrl: url,
                    bridge: bridge
                });
            }
        }

        onCaptchaRequested: {
            navigateToCaptcha(url);
        }

        onCaptchaUrlChanged: {
            if (bridge.captchaUrl && bridge.captchaUrl.length > 0) {
                navigateToCaptcha(bridge.captchaUrl);
            }
        }
    }

    Connections {
        target: bridge
        onCaptchaRequested: {
            bridge.navigateToCaptcha(url);
        }
        onCaptchaUrlChanged: {
            if (bridge.captchaUrl && bridge.captchaUrl.length > 0) {
                bridge.navigateToCaptcha(bridge.captchaUrl);
            }
        }
        onBridgePasswordChanged: {
            if (bridge.bridgePassword && bridge.bridgePassword.length > 0) {
                if (pageStack.currentPage && pageStack.currentPage.objectName === "captchaPage") {
                    pageStack.pop();
                }
            }
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: col.height + Theme.paddingLarge

        Timer {
            id: delayedRefresh
            interval: 0
            repeat: false
            onTriggered: bridge.refreshStatus()
        }

        Timer {
            id: openAboutTimer
            interval: 0
            repeat: false
            onTriggered: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
        }

        Timer {
            id: openAccountsTimer
            interval: 0
            repeat: false
            onTriggered: pageStack.push(Qt.resolvedUrl("AccountsPage.qml"))
        }

        Timer {
            id: openHowToTimer
            interval: 0
            repeat: false
            onTriggered: pageStack.push(Qt.resolvedUrl("HowToPage.qml"))
        }

        PullDownMenu {
            MenuItem {
                text: "Accounts…"
                onClicked: openAccountsTimer.restart()
            }
            MenuItem {
                text: "Refresh status"
                onClicked: delayedRefresh.restart()
            }
            MenuItem {
                text: "How to use"
                onClicked: openHowToTimer.restart()
            }
            MenuItem {
                text: "About"
                onClicked: openAboutTimer.restart()
            }
        }

        Column {
            id: col
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: "Hydroxide"
                description: "ProtonMail Bridge for Sailfish OS"
            }

            SectionHeader { text: "Service" }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                text: "Status: " + bridge.statusText
            }

            Column {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                spacing: Theme.paddingSmall

                Row {
                    width: parent.width
                    spacing: Theme.paddingMedium
                    Button { text: "Refresh"; onClicked: bridge.refreshStatus() }
                    Button { text: "Start";   onClicked: bridge.startService() }
                }
                Row {
                    width: parent.width
                    spacing: Theme.paddingMedium
                    Button { text: "Stop";    onClicked: bridge.stopService() }
                    Button { text: "Restart"; onClicked: bridge.restartService() }
                }
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                color: Theme.highlightColor
                wrapMode: Text.Wrap
                text: bridge.lastError
                visible: bridge.lastError.length > 0
            }

            SectionHeader { text: "Generate bridge password" }

            TextField {
                id: emailField
                width: parent.width
                label: "Proton email"
                placeholderText: "you@proton.me"
                inputMethodHints: Qt.ImhEmailCharactersOnly
            }

            TextField {
                id: passField
                width: parent.width
                label: "Proton password"
                echoMode: revealPassword ? TextInput.Normal : TextInput.Password
                rightItem: IconButton {
                    id: showPass
                    icon.source: revealPassword
                                 ? "image://theme/icon-splus-hide-password"
                                 : "image://theme/icon-splus-show-password"
                    onClicked: revealPassword = !revealPassword
                }
            }

            Row {
                x: Theme.horizontalPageMargin
                spacing: Theme.paddingMedium

                Button {
                    id: genButton
                    text: {
                        if (bridge.captchaUrl.length > 0 || bridge.captchaRequired) {
                            return "Open CAPTCHA";
                        }
                        if (bridge.busy) {
                            return "Working...";
                        }
                        return "Generate";
                    }
                    enabled: !bridge.busy || bridge.captchaUrl.length > 0 || bridge.captchaRequired
                    onClicked: {
                        if (bridge.captchaUrl.length > 0 || bridge.captchaRequired) {
                            bridge.navigateToCaptcha(bridge.captchaUrl);
                        } else {
                            bridge.authAndGetBridgePassword(emailField.text, passField.text);
                        }
                    }
                }

                BusyIndicator {
                    running: bridge.busy && !bridge.captchaRequired && bridge.captchaUrl.length === 0
                    visible: running
                    size: BusyIndicatorSize.Small
                    anchors.verticalCenter: genButton.verticalCenter
                }

                Button {
                    id: copyButton
                    text: "Copy"
                    enabled: bridge.bridgePassword.length > 0
                    onClicked: {
                        bridge.copyToClipboard(bridge.bridgePassword);
                    }
                }
            }

            // Notice card when 127.0.0.1 verification address is available
            Rectangle {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                height: captchaCol.height + Theme.paddingMedium * 2
                radius: Theme.paddingSmall
                color: Theme.rgba(Theme.highlightBackgroundColor, 0.2)
                border.color: Theme.highlightColor
                border.width: 1
                visible: bridge.captchaUrl.length > 0 || bridge.captchaRequired

                Column {
                    id: captchaCol
                    anchors.centerIn: parent
                    width: parent.width - Theme.paddingLarge * 2
                    spacing: Theme.paddingSmall

                    Label {
                        width: parent.width
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.highlightColor
                        font.bold: true
                        text: "Proton human verification required"
                    }

                    Label {
                        width: parent.width
                        font.pixelSize: Theme.fontSizeExtraSmall
                        color: Theme.primaryColor
                        text: bridge.captchaUrl.length > 0 ? ("Address: " + bridge.captchaUrl) : "Local verification server starting..."
                        wrapMode: Text.Wrap
                    }

                    Button {
                        text: "Open CAPTCHA in WebView"
                        onClicked: {
                            bridge.navigateToCaptcha(bridge.captchaUrl);
                        }
                    }
                }
            }

            // Bridge password card when replied
            Rectangle {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                height: pwCol.height + Theme.paddingMedium * 2
                radius: Theme.paddingSmall
                color: Theme.rgba(Theme.primaryColor, 0.15)
                border.color: Theme.highlightColor
                border.width: 1
                visible: bridge.bridgePassword.length > 0

                Column {
                    id: pwCol
                    anchors.centerIn: parent
                    width: parent.width - Theme.paddingLarge * 2
                    spacing: Theme.paddingSmall

                    Label {
                        width: parent.width
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.highlightColor
                        text: "Bridge password replied:"
                    }

                    Label {
                        width: parent.width
                        font.pixelSize: Theme.fontSizeMedium
                        font.bold: true
                        color: Theme.primaryColor
                        text: bridge.bridgePassword
                        wrapMode: Text.Wrap
                    }

                    Button {
                        text: "Copy Password"
                        onClicked: {
                            bridge.copyToClipboard(bridge.bridgePassword);
                        }
                    }
                }
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                wrapMode: Text.Wrap
                text: bridge.bridgePassword.length > 0
                      ? "Bridge password replied! Tap Copy and paste it in Settings → Accounts."
                      : "Enter your Proton credentials and tap Generate. When Proton prompts for verification, the local CAPTCHA address (http://127.0.0.1:...) opens directly in WebView."
            }
        }
    }
}
