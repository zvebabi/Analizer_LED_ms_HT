import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtCharts 2.3
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import filevalidator 1.0

Column {
    id: ctrlPane
    spacing: 3
    property int itemWidth: 400*app.dp
    property alias fileNameDialog_a: fileNameDlg
    property alias tableModel_a : tableModel
    property int globalSeriesCounter:0
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
//            deleteSeries.enabled     = true
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
                    app.dataWasChangedAfterSave = true ;
                    // createSeries()
                    var seriesName = qsTr("%1%2").arg(lineLabel.text).arg(globalSeriesCounter + 1);
//                    if ( globalSeriesCounter < 25) {
                        reciever.doMeasurements(seriesName);
//                    }
                    ++globalSeriesCounter;
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
                    app.relativeModeChBox_a.enabled = false
                    reciever.doMeasurements("etalon");
                }
            }
        }

    }
    TextField {
        id: lineLabel
       //text: qsTr("sample")
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
                        newName.focus = true
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
                Timer {
                    id: positionTimer
                    interval: 200
                    repeat: false
                    onTriggered: tableOfSeries.positionViewAtEnd()
                }
                onCountChanged: {
                        positionTimer.start()
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
                    //legend rename
                    customLegend.renameSeries(qsTr(graphs.series(tableOfSeries.currentItem.text).name), newName.text)
                    //dotted series rename
                    graphs.series(qsTr(graphs.series(
                                   tableOfSeries.currentItem.text).name+"_dotted")).name =
                            qsTr(newName.text + "_dotted");
                    //line series rename
                    graphs.series(tableOfSeries.currentItem.text).name =
                            newName.text;
                    //rename in datahandler
                    reciever.renameSeries(tableOfSeries.currentItem.text, newName.text)
                    //control panel rename
                    tableOfSeries.currentItem.text = newName.text;
                    newName.text = "";
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
                ToolTip.text: qsTr("Save data to "+reciever.getDataPath())
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
                    //width: newName.width
                    TextField {
                        id:fileNameTF
                        placeholderText: qsTr("File name")
                    }
                    standardButtons: StandardButton.OK
                    onAccepted: {
                        app.dataWasChangedAfterSave = false
                        var fileNameWithTime = new Date().toLocaleString(Qt.locale("en_US"), "yyyyMMdd_HHmmss_")
                        fileNameWithTime += (fileNameTF.text +qsTr(".csv"))
                            reciever.saveDataToCSV(fileNameWithTime);
                            //console.log(path)
                            fileNameTF.text = ""
                            delay(1, fileNameDlg.close);
                            showPopupTips(qsTr("Data saved to: \n"
                                        + reciever.getDataPath()
                                        + fileNameWithTime),
                                     1000);
                    }
                }
                FileValidator {
                    id: validatorCSV
                    url: source1
                    treatAsImage: false
                }
                onClicked: fileNameDlg.open()
                onHoveredChanged: {
                    if (hovered) {
                        ToolTip.text = qsTr("Save data to "+reciever.getDataPath())
                    }
                }
            }
            ToolButton {
                id: saveImage
                enabled: false
                height: 1.5*48*app.dp
                width: height
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Save image to "+reciever.getDataPath())
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
                    //width: newName.width
                    TextField {
                        id:imgNameTF
                        placeholderText: qsTr("Image name")
                    }
                    standardButtons: StandardButton.OK
                    onAccepted: {
                        var stringWithTime = new Date().toLocaleString(Qt.locale("en_US"), "yyyyMMdd_HHmmss_")
                        var path = reciever.getDataPath()
                                 + stringWithTime
                                 + imgNameTF.text
                                 + ".png";
                        var pathH = reciever.getDataPath()
                                  + stringWithTime
                                  + imgNameTF.text
                                  + "_hist.png";
                        //validatorIMG.url = "file:///" + path
                        //if (validatorIMG.fileValid === false) {
                            colForSnap.update();
                            colForSnap.grabToImage(function(result) {
                                    result.saveToFile(path); 
                                });
                            console.log(path)
                            imgNameTF.text = ""
                            delay(1, imgNameDlg.close);
                            showPopupTips(qsTr("Image saved to: \n" + path),
                                          1000);
                        //}
                        //else { //if file exist
                        //    imgNameTF.text = ""
                        //    delay(1, imgNameDlg.open);
                        //    showPopupTips(qsTr("Error: image exists! Choose another name"),
                        //                  1000);
                        //}
                    }
                }
                FileValidator {
                    id: validatorIMG
                    //url: source1
                    treatAsImage: true
                }
                onClicked: imgNameDlg.open()
                onHoveredChanged: {
                    if (hovered) {
                        ToolTip.text = qsTr("Save image to "+reciever.getDataPath())
                    }
                }
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
                    //graphs.zoomIn()
                    var newHeight  = graphs.plotArea.height/2;
                    graphs.zoomIn(Qt.rect(graphs.plotArea.x, graphs.plotArea.y + newHeight/2, graphs.plotArea.width, newHeight))
                    //console.log(graphs.minRngY)
                    //console.log(graphs.maxRngY)
                    //redrawHistogram()
                    //graphs.minRngY = minRngYY
                    //graphs.maxRngY = maxRngYY
                    //axisX.min = graphs.minRngX
                    //axisX.max = graphs.maxRngX
                    //axisY.min = graphs.minRngY*0.97
                    //axisY.max = graphs.maxRngY*1.03
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
                    //graphs.zoomOut()
                    graphs.zoomReset();
                    axisY.min = graphs.minRngY;
                    axisX.min = graphs.minRngX;
                    axisX.max = graphs.maxRngX;
                    axisY.max = graphs.maxRngY;
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
                        //dotted series start
                        graphs.series( qsTr(graphs.series(tableOfSeries.currentItem.text).name+"_dotted")
                                        ).visible = true;
                        //dotted series end

                    //recalc range
                      for (var j = 0; j < graphs.series(tableOfSeries.currentItem.text
                                                       ).count; j++ ) {
                        if (graphs.series(tableOfSeries.currentItem.text).at(j).y < minRngYY || minRngYY === 0){
                          minRngYY = graphs.series(tableOfSeries.currentItem.text).at(j).y;
                        }
                        if (graphs.series(tableOfSeries.currentItem.text).at(j).y > maxRngYY || maxRngYY === 0){
                          maxRngYY = graphs.series(tableOfSeries.currentItem.text).at(j).y;
                        }
                      }
                    } else {
                      //hide line from graph
                      graphs.series(tableOfSeries.currentItem.text).visible = false;
                      //hide dotted from graph
                      graphs.series(qsTr(graphs.series(tableOfSeries.currentItem.text).name+"_dotted")).visible = false;
                    }
                  }
                  //
                  redrawLegend();
                  redrawHistogram();
                  graphs.minRngY = minRngYY  - ( maxRngYY - minRngYY ) * 0.1
                  graphs.maxRngY = maxRngYY*1.03//*1.1
                  axisY.min = graphs.minRngY
                  axisY.max = graphs.maxRngY
                  axisX.min = graphs.minRngX;
                  axisX.max = graphs.maxRngX;
                }
            }
            ToolButton {
                id: deleteSeries
                enabled: true
                height: 1.5*48*app.dp
                width: height
                //anchors.left: titleText.right
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
                    property var deleteDone : false;
                    onAccepted: {
                        if ( !deleteDone ) {
                            console.log("in deleting 1");
                            for(var i = tableOfSeries.count-1; i>=0 ; i--) {
                                console.log("in deleting 1 cur index" + i );
                                tableOfSeries.currentIndex = i;
                                console.log("tableOfSeries.currentItem.checked: " + tableOfSeries.currentItem.checked)
                                if (tableOfSeries.currentItem.checked) {
                                    //remove from legend
                                    customLegend.removeSeries(tableOfSeries.currentItem.text);
                                    //remove from datahandler
                                    reciever.deleteSeries( tableOfSeries.currentItem.text );
                                    //remove dotted series
                                    graphs.removeSeries(graphs.series(qsTr(graphs.series(
                                                   tableOfSeries.currentItem.text).name+"_dotted")));
                                    //remove lineseries
                                    graphs.removeSeries(graphs.series(tableOfSeries.currentItem.text));
                                    //remove from control legend
                                    tableModel.remove(i);

                                    console.log("Series: " +
                                                tableOfSeries.currentItem.text +
                                                " deleted.");
                                }
                            }
                            console.log("recover checks")
                            //check-on all non-deleted lines
                            for(i = tableOfSeries.count-1; i>=0 ; i--) {
                                tableOfSeries.currentIndex = i;
                                tableOfSeries.currentItem.checked = true
                            }
                            deleteDone = true;
                        } else {
                            console.log("skip doubled call");
                            deleteDone = false
                        }

                        //redrawHistogram()
                    }
                }
                onClicked: { messageDialog.setVisible(true) }
            }
        }
    }
    function createSeries() {
        var colorList = [
                    "#F44336", "#673AB7", "#03A9F4", "#4CAF50", "#FFEB3B", "#FF5722",
                    "#E91E63", "#3F51B5", "#00BCD4", "#8BC34A", "#FFC107",
                    "#9C27B0", "#2196F3", "#009688", "#CDDC39", "#FF9800"
                ]
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
        var seriesName = qsTr(graphs.numSeries + "_"
                            + lineLabel.text)
        graphs.createSeries(ChartView.SeriesTypeSpline,
                            seriesName,
                            axisX, axisY);
//        reciever.doMeasurements(graphs.series(seriesName));
//dotted series start
        var seriesNameDotted = qsTr(seriesName + "_dotted")
        var series = graphs.createSeries(ChartView.SeriesTypeScatter,
                            seriesNameDotted,
                            axisX, axisY);
//        graphs.legend.markers(series)[0].setVisible(false);
        graphs.series(seriesName).color = colorList[
                    ( graphs.numSeries - 1) % colorList.length ]//"#B71C1C"
        series.color = graphs.series(seriesName).color;
        series.markerSize = 7;
        customLegend.addSeries(seriesName,series.color)
//dotted series end
        reciever.doMeasurements(graphs.series(seriesName), false, graphs.series(seriesNameDotted));
        tableModel.append({
           "name": seriesName,
           "isChecked": true,
           "seriesColor": graphs.series(seriesName).color.toString() })
    }
}
