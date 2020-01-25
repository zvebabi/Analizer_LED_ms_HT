import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.2


Item {
    id: menu

    signal menuItemClicked( string item, string page )
    property alias currentItem: listViewMenu.currentIndex
    property alias quitDialogCustom: exitDialog

    ListModel {
        id: modelMenu
        ListElement {
            item: "chart_view"
//            icon: "qrc:/images/icon_game.png"
            page: "ChartView.qml"
        }
        ListElement {
            item: "histogram_view"
//            icon: "qrc:/images/icon_game.png"
            page: "ChartView.qml"
        }
        ListElement {
            item: "settings"
//            icon: "qrc:/images/icon_settings.png"
            page: "Settings.qml"
        }
        ListElement {
            item: "about"
//            icon: "qrc:/images/icon_info.png"
            page: "PageAbout.qml"
        }
//        ListElement {
//            item: "dataDialog"
////            icon: "qrc:/images/icon_info.png"
//            page: "DataDialog.qml"
//        }
    }

    function textItemMenu( item )
    {
        var textReturn = ""
        switch( item ) {
        case "chart_view":
            textReturn = qsTr("Graph View")
            break;
        case "histogram_view":
            textReturn = qsTr("Histogram View")
            break;
        case "settings":
            textReturn = qsTr("Settings")
            break;
        case "about":
            textReturn = qsTr("About")
            break;
        case "dataDialog":
            textReturn = "dataDialog"
            break;
        }
        return textReturn
    }

    Rectangle {
        id: logoWtapper
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        width: parent.width
        height: width*0.445
        color: "white"//palette.primary //"#3078fe" //this color is equal to the background of imgLogo
        clip: true
        Image {
            id: imgLogo
            source: "qrc:/images/logo.png"
            height: parent.height
            width: parent.width
            antialiasing: true
            smooth: true
            anchors.top: parent.top
            anchors.left: parent.left
            opacity: 0.9
        }
    }
//    Image {
//        id: imgShadow
//        anchors.top: logoWtapper.bottom
//        anchors.left: parent.left
//        anchors.right: parent.right
//        height: 10*app.dp
//        z: 4
//        source: "qrc:/images/shadow_title.png"
//    }
    ListView {
        id: listViewMenu
        anchors.top: logoWtapper.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: quitButton.top
//        height:wrapperItem.height * modelMenu.count
        clip: true
        model: modelMenu
        delegate: componentDelegate
    }
    Component {
        id: componentDelegate

        Rectangle {
            id: wrapperItem
            height: 50*app.dp
            width: parent.width
            color: wrapperItem.ListView.isCurrentItem || ma.pressed ? palette.currentHighlightItem : "transparent"
//            Image {
//                id: imgItem
//                anchors.verticalCenter: parent.verticalCenter
//                anchors.left: parent.left
//                anchors.leftMargin: 0.5*imgItem.width
//                height: parent.height*0.5
//                width: height
//                source: icon
//                visible: icon != ""
//                smooth: true
//                antialiasing: true
//            }
            Label {
                id: textItem
                anchors.verticalCenter: parent.verticalCenter
//                anchors.left: imgItem.right
                x: 10
                anchors.leftMargin: 0.7*parent.height*0.5
                text: textItemMenu( item )
                font.pixelSize: parent.height*0.4
                color: wrapperItem.ListView.isCurrentItem ? palette.darkPrimary : palette.primaryText
            }
            MouseArea {
                id: ma
                anchors.fill: parent
                enabled: app.menuIsShown
                onClicked: {
                    menu.menuItemClicked( item, page )
                    listViewMenu.currentIndex = index
                }
            }
        }

    }
    Rectangle {
        id:quitButton
//        anchors.top: listViewMenu.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: parent.width
        height: 50*app.dp
//        color: "transparent"
        color: ma1.pressed ? palette.currentHighlightItem : "transparent"
        Label {
            id: quitBtnLbl
            anchors.verticalCenter: parent.verticalCenter
//            anchors.left: imgItem.right
            x: 10
            anchors.leftMargin: 0.7*parent.height*0.5
            text: qsTr("Quit")
            font.pixelSize: parent.height*0.4
            color: palette.primaryText
        }
        MessageDialog {
            id: exitDialog
            title: "Quit?"
            text: qsTr("Do you want to save dataset before quit?")
            standardButtons: StandardButton.Yes | StandardButton.No | StandardButton.Cancel
            icon: StandardIcon.Question
            onYes: {
                var backupFileName = new Date().toLocaleString(Qt.locale("en_US"), "yyyyMMdd_HHmmss'_dataset.csv'")

                var path = reciever.getDataPath() + backupFileName
                tipsWithPath.showedText = qsTr("Data saved to:\n" + path)
                tipsWithPath.open()
                reciever.saveDataToCSV(backupFileName)
                waiter.running = true
            }
            onNo: Qt.quit();
            onReset: console.log("cancel");
        }
        Timer {
            id:waiter
            interval: 1000; running: false; repeat: false
            onTriggered: Qt.quit()
        }
        MouseArea {
            id: ma1
            anchors.fill: parent
            enabled: app.menuIsShown
            onClicked: {
                exitDialog.setVisible(true)
            }
        }
    }
}

