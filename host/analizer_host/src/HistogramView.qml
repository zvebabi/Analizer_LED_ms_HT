import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtCharts 2.3
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import "QMLs"

Item {
    //save names here
    property variant allSeriesName
    property alias p_bG: barGraphs
    property alias p_bS: barSeries
    Connections {
        target: reciever
        onAdjustAxis: {
//            barGraphs.minRngY = minRng.y - (maxRng.y-minRng.y)*0.1
//            barGraphs.maxRngY = maxRng.y
//            axisY.min = 0
//            axisY.max = barGraphs.maxRngY*1.1
//            axisY.titleText = app.yAxisName
        }
    }
//    RowLayout {
//        spacing: 1
//        anchors.fill: parent
//        anchors.margins: 10

        ChartView {
            id: barGraphs
//            title: "Result"
            anchors.fill: parent
            antialiasing: true
            legend: false
            property int numSeries : 0 //current number of graphs
            property real minRngY: 0.0
            property real maxRngY: 0.0

            BarCategoryAxis {
                id: axisXbar
                objectName: "axisX"
                titleText: qsTr("Wavelength(um)")
            }
//            ValueAxis {
//                id: axisYbar
//                objectName: "axisY"
//                titleText: qsTr("Absorbance")
//                min: 0
//                max:0
//                tickCount: 5
//                minorTickCount: 4
//            }
            BarSeries {
                name: qsTr("barSeries")
                id: barSeries
                axisX: axisXbar
            }
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: 320
            Layout.minimumHeight: 240

        }
//        ChartViewEditBar {
//        }
//    }

//    function createAxis(min, max) {
//        // The following creates a ValueAxis object that can be then
//        //set as a x or y axis for a series
//        return Qt.createQmlObject("import QtQuick 2.7;
//                                   import QtCharts 2.7;
//                                   ValueAxis { min: "
//                                  + min + "; max: " + max + " }", graphs);
//    }
}

