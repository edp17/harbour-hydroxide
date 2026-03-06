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
            spacing: Theme.paddingSmall

            PageHeader {
                title: qsTr("About")
            }

            // App title
            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Hydroxide UI")
                font.pixelSize: Theme.fontSizeLarge
                font.bold: true
                color: Theme.highlightColor
            }

            // Subtitle
            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("(Helper for Hydroxide service)")
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
            }

            Image {
                source: "/usr/share/icons/hicolor/256x256/apps/harbour-hydroxide-ui.png"
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width * 0.22
                height: width
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            // Version
            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Version 1.0")
                font.pixelSize: Theme.fontSizeMedium
                color: Theme.secondaryColor
            }

            Item { width: 1; height: Theme.paddingLarge }

            Column {
                width: parent.width
                spacing: Theme.paddingMedium

                SectionHeader {
                    text: qsTr("Credits")
                }

                Label {
                    x: Theme.horizontalPageMargin
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width - 2 * x
                    wrapMode: Text.Wrap
                    color: Theme.secondaryColor
                    text: qsTr("Hydroxide is originally developed by emersion and contributors.")
                }

                ButtonLayout {
                    Button {
                        text: qsTr("Hydroxide GitHub")
                        onClicked: Qt.openUrlExternally("https://github.com/emersion/hydroxide")
                    }
                }

                Label {
                    x: Theme.horizontalPageMargin
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width - 2 * x
                    wrapMode: Text.Wrap
                    color: Theme.secondaryColor
                    text: qsTr("Hydroxide upstream is MIT licensed.")
                }



                SectionHeader {
                    text: qsTr("This app")
                }

                Label {
                    x: Theme.horizontalPageMargin
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width - 2 * x
                    wrapMode: Text.Wrap
                    color: Theme.highlightColor
                    text: qsTr("A Sailfish OS helper UI for the local Hydroxide Proton Mail bridge service.")
                }

                Label {
                    x: Theme.horizontalPageMargin
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width - 2 * x
                    wrapMode: Text.Wrap
                    color: Theme.highlightColor
                    text: qsTr("\nThis project consists of two parts:")
                }

                Label {
                    x: Theme.horizontalPageMargin
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width - 2 * x
                    wrapMode: Text.Wrap
                    color: Theme.highlightColor
                    text: qsTr("1. harbour-hydroxide:\n the service package providing the local IMAP/SMTP bridge")
                }

                Label {
                    x: Theme.horizontalPageMargin
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width - 2 * x
                    wrapMode: Text.Wrap
                    color: Theme.highlightColor
                    text: qsTr("2. harbour-hydroxide-ui:\n this Sailfish app for starting the service, generating bridge passwords, and managing accounts")
                }


                // Author
                Label {
                    width: parent.width - Theme.paddingLarge * 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeSmall
                    text: qsTr("\nDeveloped by: edp17")
                    color: Theme.secondaryColor
                }

                // License + credits
                Label {
                    width: parent.width - Theme.paddingLarge * 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeSmall
                    text: qsTr("This project is licensed under GNU GPL 3.0 or later.\nCopyright (c) 2026 edp17.\nIcons and graphics created by edp17.")
                    color: Theme.secondaryColor
                }

                // Source link
                Button {
                    text: qsTr("Source code")
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: Math.min(parent.width - Theme.paddingLarge * 2, Theme.buttonWidthLarge)
                    onClicked: Qt.openUrlExternally("https://github.com/edp17/harbour-hydroxide")
                }

                Item { width: 1; height: Theme.paddingLarge }
            }
        }
    }
}
