import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

import QtQuick.Controls.Material 2.1

ApplicationWindow {

    visible: true
    width: 960
    height: 540
    minimumHeight: 270
    minimumWidth: 480


    title: qsTr("Analizer v0.1")

    Material.theme: Material.Light
    Material.primary: Material.BlueGray
    Material.accent: Material.Teal

    SwipeView {
       id: view

       currentIndex: bar.currentIndex
       anchors.fill: parent
       ChartView {
       }
       Settings {
       }
       Item{
       }
    }
    TabBar {
        id: bar
        width: parent.width
        currentIndex: view.currentIndex
        TabButton {
           text: qsTr("Charts")
        }
        TabButton {
           text: qsTr("Settings")
        }
        TabButton {
           text: qsTr("N/A")
        }
//        position: parent.footer()
    }

}
