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
