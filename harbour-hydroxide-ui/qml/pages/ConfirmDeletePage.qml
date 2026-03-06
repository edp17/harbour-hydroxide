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
