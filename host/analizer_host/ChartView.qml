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
            axisX.min = minRng.x - (maxRng.x-minRng.x)*0.05
            axisX.max = maxRng.x + (maxRng.x-minRng.x)*0.05
            axisY.min = minRng.y - (maxRng.y-minRng.y)*0.05
            axisY.max = maxRng.y + (maxRng.y-minRng.y)*0.05
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
//            BarCategoryAxis {
//                id: axisX
//                objectName: "axisX"
//                categories: ["Jan", "Feb", "Mar", "Apr", "May", "Jun" ]

//            }
            ValueAxis {
                id: axisX
                objectName: "axisX"
                titleText: qsTr("Wavelength(nm)")
                min: 0
                max: 100
                tickCount: 10
                minorTickCount: 4
            }
            ValueAxis {
                id: axisY
                objectName: "axisY"
                titleText: qsTr("Absorbance")
                min: 0
                max:100
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

