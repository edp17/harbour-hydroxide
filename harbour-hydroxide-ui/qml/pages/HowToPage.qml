import QtQuick 2.2
import Sailfish.Silica 1.0

Page {
    id: page
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width

            PageHeader {
                title: qsTr("How to use")
            }

            Column {
                width: parent.width
                spacing: Theme.paddingMedium

                SectionHeader {
                    text: qsTr("1. Start the service")
                }

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * x
                    wrapMode: Text.Wrap
                    color: Theme.highlightColor
                    text: qsTr("Open the app and tap Start. The service status should become active.")
                }

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * x
                    wrapMode: Text.Wrap
                    color: Theme.secondaryColor
                    text: qsTr("The bridge listens locally on:\n• IMAP: 127.0.0.1:1143\n• SMTP: 127.0.0.1:1025")
                }

                SectionHeader {
                    text: qsTr("2. Generate a bridge password")
                }

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * x
                    wrapMode: Text.Wrap
                    color: Theme.highlightColor
                    text: qsTr("On the main page:\n• enter your Proton email address\n• enter your Proton password\n• tap Generate\n• tap Copy to copy the generated bridge password")
                }

                SectionHeader {
                    text: qsTr("3. Add the account in Settings")
                }

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * x
                    wrapMode: Text.Wrap
                    color: Theme.highlightColor
                    text: qsTr("Go to Settings → Accounts → Add account → Other email.")
                }

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * x
                    wrapMode: Text.Wrap
                    color: Theme.secondaryColor
                    text: qsTr("Incoming (IMAP)\n• Server: localhost (or 127.0.0.1)\n• Port: 1143\n• Security: None\n• Username: your full Proton email\n• Password: the generated bridge password")
                }

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * x
                    wrapMode: Text.Wrap
                    color: Theme.secondaryColor
                    text: qsTr("Outgoing (SMTP)\n• Server: localhost (or 127.0.0.1)\n• Port: 1025\n• Security: None\n• Username: your full Proton email\n• Password: the generated bridge password")
                }

                SectionHeader {
                    text: qsTr("Notes")
                }

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * x
                    wrapMode: Text.Wrap
                    color: Theme.secondaryColor
                    text: qsTr("Use 127.0.0.1 if localhost doesn't work. If you remove an account in this app, you may also want to remove it in Settings → Accounts.")
                }

                Item { width: 1; height: Theme.paddingLarge }
            }
        }
    }
}
