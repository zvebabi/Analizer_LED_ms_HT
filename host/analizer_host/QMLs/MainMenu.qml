import QtQuick 2.5
import QtQuick.Controls 1.4


Item {
    id: menu

    signal menuItemClicked( string item, string page )
    property alias currentItem: listViewMenu.currentIndex

    ListModel {
        id: modelMenu
        ListElement {
            item: "chart_view"
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
        ListElement {
            item: "dataDialog"
//            icon: "qrc:/images/icon_info.png"
            page: "dataDialog"
        }
    }

    function textItemMenu( item )
    {
        var textReturn = ""
        switch( item ) {
        case "chart_view":
            textReturn = qsTr("ChartView")
            break;
        case "settings":
            textReturn = qsTr("Settings")
            break;
        case "about":
            textReturn = qsTr("About")
            break;
        case "log":
            textReturn = "Log"
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
        height: width*0.2
        color: palette.primary //"#3078fe" //this color is equal to the background of imgLogo
        clip: true
//        Image {
//            id: imgLogo
//            source: "qrc:/images/background.jpg"
//            height: parent.height
//            width: parent.width
//            antialiasing: true
//            smooth: true
//            anchors.top: parent.top
//            anchors.left: parent.left
//            opacity: 0.5
//        }
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
        anchors.bottom: parent.bottom
        clip: true
        model: modelMenu
        delegate: componentDelegate
    }

    Component {
        id: componentDelegate

        Rectangle {
            id: wrapperItem
            height: 30*app.dp
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
                font.pixelSize: parent.height*0.3
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
}

