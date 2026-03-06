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
