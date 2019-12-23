import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtCharts 2.2
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import "QMLs"

Item {
    //save names here
    property variant allSeriesName
    property alias editBar_a: editBar
    property var colorList : [
            "#F44336", "#673AB7", "#03A9F4", "#4CAF50", "#FFEB3B", "#FF5722",
            "#E91E63", "#3F51B5", "#00BCD4", "#8BC34A", "#FFC107",
            "#9C27B0", "#2196F3", "#009688", "#CDDC39", "#FF9800"
        ];
    Connections {
        target: reciever
        onUpdateDrawer: {
//            graphs.removeAllSeries();
            console.log("updateDrawer received")
            adjustAxis(minRng, maxRng);
            console.log("seriesQty: " + seriesQty)
            for (var lineIdx in legend) {
                console.log("Update series #", lineIdx, " with name ", legend[lineIdx])
                if ( graphs.series(legend[lineIdx]) ) //skip if series already added
                    continue;
                var seriesToFill = graphs.createSeries(ChartView.SeriesTypeSpline,
                                          legend[lineIdx],
                                          axisX, axisY);
                var line_color = colorList[ lineIdx % colorList.length ];
                seriesToFill.color = line_color;
                reciever.updateSeries(seriesToFill, legend[lineIdx]);
                editBar.tableModel_a.append({
                   "name": legend[lineIdx],
                   "isChecked": true,
                   "seriesColor": line_color })
                //update dotted series
                var seriesToFill_dotted = graphs.createSeries(ChartView.SeriesTypeScatter,
                                          legend[lineIdx] + "_dotted",
                                          axisX, axisY);
                seriesToFill_dotted.color = line_color;
                seriesToFill_dotted.markerSize = 7;
                reciever.updateSeries(seriesToFill_dotted, legend[lineIdx]);
            }

            console.log("redrawHistogram begin");
            redrawHistogram();
            console.log("redrawHistogram end");
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
        Rectangle {
            id: colForSnap
            anchors.left: app.left
            anchors.top: parent.top
            width: app.width - editBar.itemWidth
            height: app.height - app.menuBarHeight
            Rectangle {
                id: graphsRect
                width: parent.width //- editBar.itemWidth
                height: app.height - 2*app.menuBarHeight
                anchors.right: editBar.left
                anchors.left: parent.left
                anchors.top: parent.top
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
                        tickCount: 13
                        minorTickCount: 3
                        labelFormat: "%.1f"
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
                width: parent.width
                height: 2*app.menuBarHeight
//                anchors.horizontalCenter: parent.horizontalCenter
                anchors.right: editBar.left
                anchors.left: parent.left
                anchors.top: graphsRect.bottom
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
    function adjustAxis(minRng, maxRng) {
        graphs.minRngX = (Math.floor((minRng.x - (maxRng.x-minRng.x)*0.02)*10))/10
        graphs.maxRngX = (Math.ceil((maxRng.x + (maxRng.x-minRng.x)*0.02)*10))/10
        graphs.minRngY = minRng.y - (maxRng.y-minRng.y)*0.1
        graphs.maxRngY = maxRng.y * 1.03
        axisX.min = graphs.minRngX
        axisX.max = graphs.maxRngX
        axisY.min = graphs.minRngY
        axisY.max = graphs.maxRngY

        barAxisY.min = 0
        barAxisY.max = graphs.maxRngY*1.1
    }

    function redrawHistogram() {
        //remove all existing sets of data
        mainBarSeries.clear()
        var _axis = [];
        //fill axisX
        if ( graphs.count > 0 ) {
            var line__ = graphs.series(0);
            for (var _ind = 0; _ind < line__.count; _ind++) { //go through all points
                _axis.push(qsTr("%1").arg(line__.at(_ind).x));
            }
        }
        barAxisX.categories = _axis
        barAxisX.visible = true
        //fill histogram again. and set visible all
        //+=2 because we have 2 kind of series(spline and dotted) with same data
        for ( var i = 0; i < graphs.count ; i+=2 ) {
            //set visible only checked

            if (graphs.series(i).visible) {
                var lineS = graphs.series(i);
                var label = lineS.name; // name for barset
                var colorS = lineS.color; //color for barset
                //fill data values
                var dataS = [];
                for (var ind = 0; ind < lineS.count; ind++) { //go through all points
                    dataS.push(lineS.at(ind).y);
                }
                var newBarSet = mainBarSeries.append(label, dataS);
                newBarSet.color = colorS;
            }
        }

    }
}

