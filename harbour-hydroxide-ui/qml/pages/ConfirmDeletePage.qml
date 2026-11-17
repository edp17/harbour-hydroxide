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

    property var users: []
    property var actions: []

    BridgeHelper { id: bridge }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: col.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem { text: "Cancel"; onClicked: pageStack.pop() }
        }

        Column {
            id: col
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader { title: "Confirm removal" }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                wrapMode: Text.Wrap
                text: "Selected account(s):\n" + (users.length > 0 ? users.join("\n") : "(none)")
            }

            SectionHeader { text: "Actions" }

            Repeater {
                model: actions
                delegate: Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2*x
                    wrapMode: Text.Wrap
                    font.pixelSize: Theme.fontSizeSmall

                    text: (modelData.indexOf("AUTHJSON:") === 0)
                          ? ("Remove from auth.json: " + modelData.substring("AUTHJSON:".length))
                          : modelData
                }
            }

            ViewPlaceholder {
                enabled: actions.length === 0
                text: "Nothing matched for deletion."
            }

            Row {
                x: Theme.horizontalPageMargin
                spacing: Theme.paddingMedium

                Button {
                    text: "Cancel"
                    onClicked: pageStack.pop()
                }

                Button {
                    text: "Delete"
                    enabled: users.length > 0 && actions.length > 0
                    onClicked: {
                        bridge.removeUsers(users)
                        pageStack.pop()
                    }
                }
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                wrapMode: Text.Wrap
                color: Theme.highlightColor
                text: bridge.lastError
                visible: bridge.lastError.length > 0
            }
        }
    }
}
