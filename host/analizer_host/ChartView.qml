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
//            barGraphs.visible = barGraphs.visible ? false :true
            graphs.minRngX = minRng.x - (maxRng.x-minRng.x)*0.02
            graphs.maxRngX = maxRng.x + (maxRng.x-minRng.x)*0.02
            graphs.minRngY = minRng.y - (maxRng.y-minRng.y)*0.1
            graphs.maxRngY = maxRng.y
            axisX.min = graphs.minRngX
            axisX.max = graphs.maxRngX
            axisY.min = 0
            axisY.max = graphs.maxRngY*1.1

            barAxisY.min = 0
            barAxisY.max = graphs.maxRngY*1.1
//            console.log("adjust axis")
//            var srcSeries = graphs.series(graphs.numSeries-1)
//            var serName = qsTr(srcSeries.name + "_dot")
//            var scatter = graphs.createSeries(ChartView.SeriesTypeScatter,
//                                   srcSeries.name,
//                                   axisX, axisY);
//            scatter.color = srcSeries.color
////            scatter.name = srcSeries.name
//            for (var p =0; p <  srcSeries.count; p++) {
//                scatter.append(srcSeries.at(p).x, srcSeries.at(p).y)
//            }
        }
        onUpdateBarSeries: {
            mainBarSeries.append(_label, _data)
            mainBarSeries.at(mainBarSeries.count-1).color = _color
            barAxisX.categories = _axis
            barAxisX.visible = true

        }
        onSendDebugInfo: {
            showPopupTips(qsTr(data), time)
        }
    }

    RowLayout {
        spacing: 1
        anchors.fill: parent
        anchors.margins: 10
        Column {
            id: colForSnap
            spacing:0
            anchors.fill: parent
//            anchors.margins: 10
            Rectangle {
                width: parent.width - editBar.itemWidth
                height: app.height - 1.5*app.menuBarHeight
                anchors.right: editBar.left
                anchors.left: parent.left
                anchors.top: parent.top - app.menuBarHeight

                ChartView {
                    id: graphs
                    anchors.fill: parent
                    antialiasing: true
                    legend.visible: false
                    visible: mainMenu.currentItem == 0 ? true : false // 0 - line
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
                        titleText: app.yAxisName
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
                    visible: mainMenu.currentItem == 1 ? true : false //1-histogram
                    property int numSeries : 0 //current number of graphs
                    property real minRngX: 0.0
                    property real maxRngX: 0.0
                    property real minRngY: 0.0
                    property real maxRngY: 0.0
                    BarCategoryAxis {
                        id: barAxisX
                        objectName: "barAxisX"
                        titleText: qsTr("Wavelength(um)")
                        visible: false
                    }

                    ValueAxis {
                        id: barAxisY
                        objectName: "barAxisY"
                        titleText: app.yAxisName
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
            CustomLegend {
                id: customLegend
//                visible: app.ctmLegendVisibility
                width: parent.width - editBar.itemWidth
                height: app.menuBarHeight
//                anchors.horizontalCenter: parent.horizontalCenter
                anchors.right: editBar.left
                anchors.left: parent.left
                anchors.top: graphs.bottom
//                onEntered: chartViewSelector.highlightSeries(seriesName);
//                onExited: chartViewSelector.highlightSeries("");
//                onSelected: chartViewSelector.selectSeries(seriesName);
            }
        }
        ChartViewEditBar {
            id: editBar
            anchors.right: parent.right

        }

    }
    Timer {
        id: timer1
    }
    Timer {
        id: timer2
    }
    function delay(delayTime, cb) {
        timer1.interval = delayTime;
        timer1.repeat = false;
        timer1.triggered.connect(cb);
        timer1.start();
    }
    function createAxis(min, max) {
        // The following creates a ValueAxis object that can be then
        //set as a x or y axis for a series
        return Qt.createQmlObject("import QtQuick 2.7;
                                   import QtCharts 2.7;
                                   ValueAxis { min: "
                                  + min + "; max: " + max + " }", graphs);
    }
    function showPopupTips(text, dTime) {
        tipsWithPath.showedText = qsTr(text)
        tipsWithPath.open()
        delay(dTime !== undefined ? dTime : 300, tipsWithPath.close)
    }
}

