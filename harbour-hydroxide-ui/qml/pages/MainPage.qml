/*
    Copyright (C) 2026 edp17 and chatGPT

    This file is part of harbour-hydroxide-ui.

    The harbour-hydroxide-ui is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The harbour-hydroxide-ui is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the harbour-hydroxide-ui. If not, see <http://www.gnu.org/licenses/>.
*/
import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.hydroxide 1.0

Page {
    id: page
    allowedOrientations: Orientation.All
    
    property bool revealPassword: false

    BridgeHelper { id: bridge }

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

            PageHeader { title: "Hydroxide" }

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
                    text: bridge.busy ? "Working..." : "Generate"
                    enabled: !bridge.busy
                    onClicked: bridge.authAndGetBridgePasswordPty(emailField.text, passField.text)
                }

                BusyIndicator {
                    running: bridge.busy
                    visible: bridge.busy
                    size: BusyIndicatorSize.Small
                    anchors.verticalCenter: genButton.verticalCenter
                }

                Button {
                    id: copyButton
                    text: "Copy"
                    enabled: bridge.bridgePassword.length > 0
                    onClicked: {
                        bridge.copyToClipboard(bridge.bridgePassword)
                        // Optional: clear password field for safety
                        // passField.text = ""
                    }
                }
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                wrapMode: Text.Wrap
                text: bridge.bridgePassword.length > 0
                      ? ("Bridge password:\n" + bridge.bridgePassword)
                      : "Generate a bridge password, then tap Copy and paste it in Settings → Accounts."
            }
        }
    }
}
