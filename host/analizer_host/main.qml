import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

import QtQuick.Controls.Material 2.1

ApplicationWindow {

    visible: true
    width: 640
    height: 480
    title: qsTr("Сложение двух чисел")

    Material.theme: Material.Light
    Material.primary: Material.BlueGray
    Material.accent: Material.Teal

    signal qmlSignal(string msg)
    signal getPorts()
    signal initDevice(string portName);

    SwipeView {
       id: view

       currentIndex: bar.currentIndex
       anchors.fill: parent
       Settings {
       }
       ChartView {
       }
       Item{
       }
    }
    TabBar {
        id: bar
        width: parent.width
        TabButton {
           text: qsTr("Settings")
        }
        TabButton {
           text: qsTr("Charts")
        }
        TabButton {
           text: qsTr("N/A")
        }
    }

}
