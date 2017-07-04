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
            graphs.minRngX = minRng.x - (maxRng.x-minRng.x)*0.02
            graphs.maxRngX = maxRng.x + (maxRng.x-minRng.x)*0.02
            graphs.minRngY = minRng.y - (maxRng.y-minRng.y)*0.1
            graphs.maxRngY = maxRng.y
            axisX.min = graphs.minRngX
            axisX.max = graphs.maxRngX
            axisY.min = 0
            axisY.max = graphs.maxRngY*1.1
            axisY.titleText = app.yAxisName
        }
    }


    RowLayout {
        spacing: 1
        anchors.fill: parent
        anchors.margins: 10

        ChartView {
            id: graphs
//            title: "Result"
            antialiasing: true
            legend.visible: false
            property int numSeries : 0 //current number of graphs
            property real minRngX: 0.0
            property real maxRngX: 0.0
            property real minRngY: 0.0
            property real maxRngY: 0.0
//            BarCategoryAxis {
//                id: axisX
//                objectName: "axisX"
//                categories: ["Jan", "Feb", "Mar", "Apr", "May", "Jun" ]

//            }
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
        ChartViewEditBar {

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

