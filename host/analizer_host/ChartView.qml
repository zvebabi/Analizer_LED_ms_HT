import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtCharts 2.2
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import "QMLs"

Item {
    //save names here
    property variant allSeriesName

    Connections {
        target: reciever
        onAdjustAxis: {
            barGraphs.visible = barGraphs.visible ? false :true
            graphs.minRngX = minRng.x - (maxRng.x-minRng.x)*0.02
            graphs.maxRngX = maxRng.x + (maxRng.x-minRng.x)*0.02
            graphs.minRngY = minRng.y - (maxRng.y-minRng.y)*0.1
            graphs.maxRngY = maxRng.y
            axisX.min = graphs.minRngX
            axisX.max = graphs.maxRngX
            axisY.min = 0
            axisY.max = graphs.maxRngY*1.1
            axisY.titleText = app.yAxisName

            barAxisY.min = 0
            barAxisY.max = graphs.maxRngY*1.1
        }
        onUpdateBarSeries: {
            mainBarSeries.append(_label, _data)
//            mainBarSeries.append("set 1", [0, 0.2, 0.2, 0.5, 0.4, 1.5, 0.9])
            mainBarSeries.at(mainBarSeries.count-1).color = _color
            barAxisX.categories = _axis

        }
    }

    RowLayout {
        spacing: 1
        anchors.fill: parent
        anchors.margins: 10
        Rectangle {
            width: parent.width - editBar.itemWidth
            height: app.height - app.menuBarHeight
            anchors.right: editBar.left
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            ChartView {
                id: graphs
                anchors.fill: parent
                antialiasing: true
                legend.visible: false
                property int numSeries : 0 //current number of graphs
                property real minRngX: 0.0
                property real maxRngX: 0.0
                property real minRngY: 0.0
                property real maxRngY: 0.0
                ValueAxis {
                    id: axisX
                    objectName: "axisX"
                    titleText: qsTr("Wavelength(um)")
                    min: 0
                    max: 0
                    tickCount: 12
                    minorTickCount: 4
                }
                ValueAxis {
                    id: axisY
                    objectName: "axisY"
                    titleText: qsTr("Absorbance")
                    min: 0
                    max:0
                    tickCount: 5
                    minorTickCount: 4
                }
                MouseArea {
                    anchors.fill: parent
                    property int lastX: 0
                    property int lastY: 0
                    onPressed: {
                        lastX = mouse.x
                        lastY = mouse.y
                    }
                    onReleased: {
    //                    view.interactive : true
                    }
                    onPositionChanged: {
                        if (lastX !== mouse.x) {
                            graphs.scrollRight(lastX - mouse.x)
                            lastX = mouse.x
                        }
                        if (lastY !== mouse.y) {
                            graphs.scrollDown(lastY - mouse.y)
                            lastY = mouse.y
                        }
                    }
                }
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: 320
                Layout.minimumHeight: 240
            }

            ChartView {
                id: barGraphs
                anchors.fill: parent
    //            title: "Result"
                antialiasing: true
                legend.visible: false
                property int numSeries : 0 //current number of graphs
                property real minRngX: 0.0
                property real maxRngX: 0.0
                property real minRngY: 0.0
                property real maxRngY: 0.0
                BarCategoryAxis {
                    id: barAxisX
                    objectName: "barAxisX"
                    titleText: qsTr("Wavelength(um)")
                }

                ValueAxis {
                    id: barAxisY
                    objectName: "barAxisY"
                    titleText: qsTr("Absorbance")
                    min: 0
                    max:0
                    tickCount: 5
                    minorTickCount: 4
                }
                BarSeries {
                    id:mainBarSeries
                    name: "mainBarSeries"
                    axisX: barAxisX
                    axisY: barAxisY

                }

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: 320
                Layout.minimumHeight: 240

            }
        }
        ChartViewEditBar {
            id: editBar
            anchors.right: parent.right

        }

    }
    function createAxis(min, max) {
        // The following creates a ValueAxis object that can be then
        //set as a x or y axis for a series
        return Qt.createQmlObject("import QtQuick 2.7;
                                   import QtCharts 2.7;
                                   ValueAxis { min: "
                                  + min + "; max: " + max + " }", graphs);
    }
}

