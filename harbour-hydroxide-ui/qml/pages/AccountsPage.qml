import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.hydroxide 1.0

Page {
    id: page
    allowedOrientations: Orientation.All

    BridgeHelper { id: bridge }

    // email -> true
    property var selected: ({})
    property int selectedCount: 0

    function recountSelected() {
        var c = 0
        for (var k in selected) {
            if (selected[k] === true) c++
        }
        selectedCount = c
    }

    function selectedList() {
        var out = []
        for (var k in selected) {
            if (selected[k] === true) out.push(k)
        }
        return out
    }

    function clearSelection() {
        selected = ({})
        selectedCount = 0
    }

    function toggleSelection(email) {
        selected[email] = !(selected[email] === true)
        selected = selected      // force bindings to re-evaluate (JS object)
        recountSelected()
    }

    // Refresh when page becomes visible (more reliable than only onCompleted)
    onStatusChanged: {
        if (status === PageStatus.Active) {
            clearSelection()
            bridge.refreshUsers()
        }
    }

    Timer {
        id: delayedRefresh
        interval: 0
        repeat: false
        onTriggered: bridge.refreshUsers()
    }

    Item {
        anchors.fill: parent

        // Fixed bottom button bar
        Item {
            id: footer
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: Theme.itemSizeLarge

            Row {
                anchors.centerIn: parent
                spacing: Theme.paddingMedium

                Button {
                    text: "Clear selection"
                    enabled: page.selectedCount > 0
                    onClicked: page.clearSelection()
                }

                Button {
                    text: "Remove selected"
                    enabled: page.selectedCount > 0
                    onClicked: {
                        var users = page.selectedList()
                        var actions = bridge.previewRemoveUsers(users)
                        pageStack.push(Qt.resolvedUrl("ConfirmDeletePage.qml"), { users: users, actions: actions })
                        page.clearSelection()
                    }
                }
            }
        }

        SilicaListView {
            id: list
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: footer.top

            header: Column {
                width: list.width
                spacing: Theme.paddingSmall

                PageHeader {
                    title: "Accounts"
                }

                Label {
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    x: Theme.horizontalPageMargin
                    wrapMode: Text.Wrap
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                    text: "Tap an account to select it. Selected accounts can be removed with the button below."
                }

                Item {
                    width: 1
                    height: Theme.paddingSmall
                }
            }

            PullDownMenu {
                MenuItem {
                    text: "Refresh"
                    onClicked: delayedRefresh.restart()
                }
            }

            model: bridge.users

            delegate: BackgroundItem {
                width: parent.width
                contentHeight: Theme.itemSizeMedium

                Row {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2*x
                    height: parent.contentHeight
                    spacing: Theme.paddingMedium

                    IconButton {
                        // Your chosen icons:
                        icon.source: (page.selected[modelData] === true)
                                     ? "image://theme/icon-m-delete"
                                     : "image://theme/icon-m-right"
                        onClicked: page.toggleSelection(modelData)
                    }

                    Label {
                        width: parent.width - Theme.itemSizeMedium
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData
                        truncationMode: TruncationMode.Fade
                    }
                }

                onClicked: page.toggleSelection(modelData)
            }

            ViewPlaceholder {
                enabled: bridge.users.length === 0
                text: "No logged-in accounts yet.\nUse Generate on the main page."
            }
        }
    }
}
