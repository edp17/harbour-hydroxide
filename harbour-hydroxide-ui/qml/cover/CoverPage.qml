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
import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.hydroxide 1.0

CoverBackground {
    id: cover

    BridgeHelper {
        id: bridge
    }

    Component.onCompleted: bridge.refreshStatus()
    onStatusChanged: {
        if (status === Cover.Active)
            bridge.refreshStatus()
    }

    Column {
        anchors.centerIn: parent
        width: parent.width - 2 * Theme.paddingLarge
        spacing: Theme.paddingMedium

        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Hydroxide UI")
            font.pixelSize: Theme.fontSizeSmall
            font.bold: true
            truncationMode: TruncationMode.Fade
        }

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            source: "/usr/share/icons/hicolor/256x256/apps/harbour-hydroxide-ui.png"
            width: Theme.iconSizeLauncher
            height: Theme.iconSizeLauncher
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Service status")
            color: Theme.highlightColor
            font.pixelSize: Theme.fontSizeSmall
            truncationMode: TruncationMode.Fade
        }

        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: bridge.statusText
            font.pixelSize: Theme.fontSizeSmall
            font.bold: true
            color: bridge.statusText === "active" ? Theme.highlightColor : Theme.primaryColor
            truncationMode: TruncationMode.Fade
        }
    }

    CoverActionList {
        CoverAction {
            iconSource: "image://theme/icon-cover-refresh"
            onTriggered: bridge.refreshStatus()
        }
    }
}
