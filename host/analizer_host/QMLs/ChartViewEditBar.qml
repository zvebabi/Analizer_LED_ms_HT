import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtCharts 2.2
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0

Column {
    id: ctrlPane
    spacing: 3
    property int itemWidth: 400*app.dp
    signal redrawHistogram()
    Connections {
        target: reciever
        onMakeSeries: {
            createSeries()
        }
        onActivateEditBar: {
            runAnalizer.enabled      = true
            saveData.enabled         = true
            saveImage.enabled        = true
            zoomIn.enabled           = true
            zoomOut.enabled          = true
            setSeriesVisible.enabled = true
            deleteSeries.enabled     = true
        }
    }
    Rectangle
    {
        id: mainBtnHolder
        width: ctrlPane.itemWidth
        height: 120*app.dp
        anchors.top:  parent.top - app.menuBarHeight
        color: "transparent"
        Grid {
            id : chartEditMenu2
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            columns: 2
            spacing: 20
            ToolButton {
                id: runAnalizer
                enabled: false
                height: 2.3*48*app.dp
                width: height
                ToolTip.visible: hovered
                    ToolTip.text: qsTr("Measurement")
                Image {
                    id: rAa
                    anchors.centerIn: parent
                    width: parent.width
                    height: width
                    source: "qrc:/images/runAnalizer.png"
                    antialiasing: true
                    smooth: true
                }
                ColorOverlay {
                    anchors.fill: rAa
                    source: rAa
                    color: "#d000ff00"
                }
                onClicked: {
                    createSeries()
                }
            }
            ToolButton {
                id: runAnalizerExpress
                height: 2.3*48*app.dp
                width: height
                ToolTip.visible: hovered
                    ToolTip.text: app.relativeMode ?
                                      qsTr("Relative mode calibration") : qsTr("Absolute mode calibration")
                Image {
                    id: rAe
                    anchors.centerIn: parent
                    width: parent.width
                    height: width
                    source: "qrc:/images/runCalibration.png"
                    antialiasing: true
                    smooth: true
                }

                ColorOverlay {
                    anchors.fill: rAe
                    source: rAe
                    color: "#80ff0000"
                }
                onClicked: {
                    if (drawEt === true) {
                        graphs.numSeries++;
                        var seriesName = qsTr("calibration_"
                                              + graphs.numSeries)
                        graphs.createSeries(ChartView.SeriesTypeLine,
                                            seriesName,
                                            axisX, axisY);
                        tableModel.append({
                           "name": seriesName,
                           "isChecked": true,
                           "seriesColor":
                                   graphs.series(seriesName).color.toString() })
                        }
                    reciever.doMeasurements(graphs.series(seriesName),true);
                }
            }
        }

    }
    TextField {
        id: lineLabel
//        text: qsTr("sample")
        placeholderText: qsTr("Sample name")
        selectByMouse: true
        width: ctrlPane.itemWidth
        ToolTip.visible: hovered
            ToolTip.text: qsTr("Enter sample name")
    }
    Row {
        Rectangle {
            width: ctrlPane.itemWidth - chartEditMenu.width
            height: 400*app.dp
            anchors.topMargin: 50
            color: "transparent"
            ListView {
                id: tableOfSeries
                anchors.fill: parent
                model: ListModel { id: tableModel }
                delegate: CheckDelegate {
                    text: name
                    checked: isChecked
                    background: Rectangle {color: seriesColor}
                    width: ctrlPane.itemWidth - runAnalizer.width
                    height: 45*app.dp
                    onDoubleClicked: {//rename sample
                        tableOfSeries.currentIndex = index
                        renameDlg.visible = true;
                    }
                }
                focus: true
                keyNavigationEnabled: true
                ScrollIndicator.vertical: ScrollIndicator { }
                remove: Transition {
                    ParallelAnimation {
                        NumberAnimation { property: "opacity"; to: 0;
                            duration: 1000 }
                        NumberAnimation { properties: "x,y"; to: 100;
                            duration: 1000 }
                    }
                }
            }
            Dialog {
                id: renameDlg
                title: qsTr("Enter new name")
                visible: false
//                width: newName.width
                TextField {
                    id:newName
                    placeholderText: qsTr("Enter new name")
                }
                standardButtons: StandardButton.OK
                onAccepted: {
                    console.log(newName.text)
                    graphs.series(tableOfSeries.currentItem.text).name =
                            newName.text
                    tableOfSeries.currentItem.text = newName.text
                    newName.text = ""
                }
            }
        }
        Grid {
            id : chartEditMenu
            columns: 1
            spacing: 0
            ToolButton {
                id: saveData
                enabled: false
                height: 1.5*48*app.dp
                width: height
                ToolTip.visible: hovered
                    ToolTip.text: qsTr("Save data to file")
                Image {
                    anchors.centerIn: parent
                    width: parent.width
                    height: width
                    source: "qrc:/images/saveData.png"
                    antialiasing: true
                    smooth: true
                }
                Dialog {
                    id: fileNameDlg
                    title: qsTr("Enter file name")
                    visible: false
    //                width: newName.width
                    TextField {
                        id:fileNameTF
                        placeholderText: qsTr("File name")
                    }
                    standardButtons: StandardButton.OK
                    onAccepted: {
                        reciever.saveDataToCSV(fileNameTF.text + ".csv");
                        var path = reciever.getDataPath() +
                                fileNameTF.text + ".csv"
                        fileNameTF.text = ""
                        tipsWithPath.showedText = qsTr("Data saved to: \n" +
                                                       path)
                        tipsWithPath.open()
                    }
                }
                onClicked: fileNameDlg.open()
            }
            ToolButton {
                id: saveImage
                enabled: false
                height: 1.5*48*app.dp
                width: height
                ToolTip.visible: hovered
                    ToolTip.text: qsTr("Save image")
                Image {
                    anchors.fill: parent
                    source: "qrc:/images/takeSnapshot.png"
                    antialiasing: true
                    smooth: true
                }
                Dialog {
                    id: imgNameDlg
                    title: qsTr("Enter image name")
                    visible: false
    //                width: newName.width
                    TextField {
                        id:imgNameTF
                        placeholderText: qsTr("Image name")
                    }
                    standardButtons: StandardButton.OK
                    onAccepted: {
                        var path = reciever.getDataPath() +
                                imgNameTF.text + ".png";
                        var pathH = reciever.getDataPath() +
                                imgNameTF.text + "_hist.png";
                        graphs.grabToImage(function(result) {
                            result.saveToFile(path);
                        });
                        barGraphs.grabToImage(function(result) {
                            result.saveToFile(pathH);
                        });
                        console.log(path)
                        imgNameTF.text = ""
                        tipsWithPath.showedText = qsTr("Image saved to: \n" +
                                                       path)
                        tipsWithPath.open()
                    }
                }
                onClicked: imgNameDlg.open()
            }
            ToolButton {
                id: zoomIn
                enabled: false
                height: 1.5*48*app.dp
                width: height
                ToolTip.visible: hovered
                    ToolTip.text: qsTr("Zoom In")
                Image {
                    anchors.fill: parent
                    anchors.centerIn: parent
                    source: "qrc:/images/zoomIn.png"
                    antialiasing: true
                    smooth: true
                }
                onClicked: {
//                    graphs.zoomIn()
//                    graphs.zoomIn(Qt.rect(0, 0, graphs.plotArea.width, graphs.plotArea.height/2))
//                    console.log(graphs.minRngY)
//                    console.log(graphs.maxRngY)
                    var minRngYY =0;
                    var maxRngYY =0;
                    for(var i = tableOfSeries.count-1; i >= 0; i--) {
                      tableOfSeries.currentIndex = i;
                      if (tableOfSeries.currentItem.checked) {
                        graphs.series(tableOfSeries.currentItem.text
                                      ).visible = true;
                        console.log("Series: " +
                                  tableOfSeries.currentItem.text + " is off.");
                      //recalc range
                        for(var j = 0; j < graphs.series(tableOfSeries.currentItem.text
                                                         ).count; j ++ ){
                          if (graphs.series(tableOfSeries.currentItem.text).at(j).y < minRngYY || minRngYY ==0){
                            minRngYY = graphs.series(tableOfSeries.currentItem.text).at(j).y;
                          }
                          if (graphs.series(tableOfSeries.currentItem.text).at(j).y > maxRngYY || maxRngYY ==0){
                            maxRngYY = graphs.series(tableOfSeries.currentItem.text).at(j).y;
                          }
                        }
                      } else {
                        graphs.series(tableOfSeries.currentItem.text).visible =
                                                                          false;}
                    }
                    redrawHistogram()
                    graphs.minRngY = minRngYY
                    graphs.maxRngY = maxRngYY
                    axisX.min = graphs.minRngX
                    axisX.max = graphs.maxRngX
                    axisY.min = graphs.minRngY*0.97
                    axisY.max = graphs.maxRngY*1.03
                }
            }
            ToolButton {
                id: zoomOut
                enabled: false
                height: 1.5*48*app.dp
                width: height
                ToolTip.visible: hovered
                    ToolTip.text: qsTr("Zoom Out")
                Image {
                    anchors.fill: parent
                    source: "qrc:/images/zoomOut.png"
                    antialiasing: true
                    smooth: true
                }
                onClicked: {
//                    graphs.zoomOut()
                    graphs.zoomReset()
                    axisY.min = 0;
                    axisX.min = graphs.minRngX
                    axisX.max = graphs.maxRngX
                    axisY.max = graphs.maxRngY*1.03
                }
            }
            ToolButton {
                id: setSeriesVisible
                enabled: false
                height: 1.5*48*app.dp
                width: height
                ToolTip.visible: hovered
                    ToolTip.text: qsTr("Show only selected series")
                Image {
                    anchors.fill: parent
                    source: "qrc:/images/setVisible.png"
                    antialiasing: true
                    smooth: true
                }
                onClicked: {//if checked vis=true, else false
                  var minRngYY =0;
                  var maxRngYY =0;
                  for(var i = tableOfSeries.count-1; i >= 0; i--) {
                    tableOfSeries.currentIndex = i;
                    if (tableOfSeries.currentItem.checked) {
                      graphs.series(tableOfSeries.currentItem.text
                                    ).visible = true;
                      console.log("Series: " +
                                tableOfSeries.currentItem.text + " is off.");
                    //recalc range
                      for(var j = 0; j < graphs.series(tableOfSeries.currentItem.text
                                                       ).count; j ++ ){
                        if (graphs.series(tableOfSeries.currentItem.text).at(j).y < minRngYY || minRngYY ==0){
                          minRngYY = graphs.series(tableOfSeries.currentItem.text).at(j).y;
                        }
                        if (graphs.series(tableOfSeries.currentItem.text).at(j).y > maxRngYY || maxRngYY ==0){
                          maxRngYY = graphs.series(tableOfSeries.currentItem.text).at(j).y;
                        }
                      }
                    } else {
                      graphs.series(tableOfSeries.currentItem.text).visible =
                                                                        false;}
                  }
                  redrawHistogram()
                  graphs.minRngY = minRngYY
                  graphs.maxRngY = maxRngYY//*1.1
                  axisY.min = minRngYY*0.97
                  axisY.max = maxRngYY*1.03
                }
            }
            ToolButton {
                id: deleteSeries
                enabled: false
                height: 1.5*48*app.dp
                width: height
    //                anchors.left: titleText.right
                ToolTip.visible: hovered
                    ToolTip.text: qsTr("Delete selected series")
                Image {
                    anchors.fill: parent
                    source: "qrc:/images/delete.png"
                    antialiasing: true
                    smooth: true
                }
                MessageDialog {
                    id: messageDialog
                    title: "Confirm deleting of series"
                    text: "Are you really sure?"
                    standardButtons: StandardButton.Ok | StandardButton.Cancel
                    icon: StandardIcon.Warning
                    onAccepted: {
                        for(var i = tableOfSeries.count-1; i>=0 ; i--) {
                            tableOfSeries.currentIndex = i;
                            if (tableOfSeries.currentItem.checked) {
                                reciever.deleteSeries(
                                            graphs.series(
                                               tableOfSeries.currentItem.text));
                                graphs.removeSeries(
                                            graphs.series(
                                               tableOfSeries.currentItem.text));
                                tableModel.remove(i);
                                console.log("Series: " +
                                            tableOfSeries.currentItem.text +
                                            " deleted.");
                            }
                        }
                        //check-on all non-deleted lines
                        for(i = tableOfSeries.count-1; i>=0 ; i--) {
                            tableOfSeries.currentIndex = i;
                            tableOfSeries.currentItem.checked = true
                        }
                        redrawHistogram()
                    }
                }
                onClicked: { messageDialog.setVisible(true) }
            }
        }
    }
    function createSeries() {
        //in cumulative mode delete previous series, if not first
//        if(graphs.numSeries !=0 && app.cumulativeMode)
//        {
//            tableOfSeries.currentIndex = tableOfSeries.count-1;
//            reciever.deleteSeries(
//                        graphs.series(
//                           tableOfSeries.currentItem.text));
//            graphs.removeSeries(
//                        graphs.series(
//                           tableOfSeries.currentItem.text));
//            tableModel.remove(tableOfSeries.currentIndex);
//            console.log("Series: " +
//                        tableOfSeries.currentItem.text +
//                        " deleted.");
//        }

        graphs.numSeries++;
        var seriesName = qsTr(lineLabel.text + "_"
                              + graphs.numSeries)
        graphs.createSeries(ChartView.SeriesTypeSpline,
                            seriesName,
                            axisX, axisY);
//        reciever.doMeasurements(graphs.series(seriesName));
//dotted series start
        var seriesNameDotted = qsTr(seriesName + "_dotted")
        var series = graphs.createSeries(ChartView.SeriesTypeScatter,
                            seriesNameDotted,
                            axisX, axisY);
        series.color = graphs.series(seriesName).color;
//dotted series end
        reciever.doMeasurements(graphs.series(seriesName), false, graphs.series(seriesNameDotted));
        tableModel.append({
           "name": seriesName,
           "isChecked": true,
           "seriesColor": graphs.series(seriesName).color.toString() })
    }
    onRedrawHistogram: {
        //remove all existing sets of data
        mainBarSeries.clear()
        //fill histogram again. and set visible all
        for ( var i = 0; i < tableOfSeries.count ; i++ ) {
            tableOfSeries.currentIndex = i;
            //set visible only checked
            if (tableOfSeries.currentItem.checked) {
                var lineS = graphs.series(tableOfSeries.currentItem.text);
                var label = lineS.name; // name for barset
                var colorS = lineS.color; //color for barset
                //fill data values
                var dataS = [];
                for (var ind = 0; ind < lineS.count; ind++) { //go through all points
                    dataS.push(lineS.at(ind).y);
                }
//                console.log(dataS);
//                console.log(label);
//                console.log(colorS);
                var newBarSet = mainBarSeries.append(label, dataS);
                newBarSet.color = colorS;
            }
        }
    }
}
