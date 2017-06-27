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
    Connections {
        target: reciever
        onMakeSeries: {
            graphs.numSeries++;
            var seriesName = qsTr(lineLabel.text + "_"
                                  + graphs.numSeries)
            graphs.createSeries(ChartView.SeriesTypeLine,
                                seriesName,
                                axisX, axisY);
            reciever.doMeasurements(graphs.series(seriesName));
            tableModel.append({
               "name": seriesName,
               "isChecked": true,
               "seriesColor": graphs.series(seriesName).color.toString() })
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
                        NumberAnimation { property: "opacity"; to: 0; duration: 1000 }
                        NumberAnimation { properties: "x,y"; to: 100; duration: 1000 }
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
                    graphs.series(tableOfSeries.currentItem.text).name = newName.text
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
                id: runAnalizer
                height: 1.6*48*app.dp
                width: height
                ToolTip.visible: hovered
                    ToolTip.text: qsTr("Run " +app.appTitle)
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
                    graphs.numSeries++;
                    var seriesName = qsTr(lineLabel.text + "_"
                                          + graphs.numSeries)
                    graphs.createSeries(ChartView.SeriesTypeLine,
                                        seriesName,
                                        axisX, axisY);
                    reciever.doMeasurements(graphs.series(seriesName));
                    tableModel.append({
                       "name": seriesName,
                       "isChecked": true,
                       "seriesColor": graphs.series(seriesName).color.toString() })
                }
            }
            ToolButton {
                id: runAnalizerExpress
                height: 1.6*48*app.dp
                width: height
                ToolTip.visible: hovered
                    ToolTip.text: qsTr("Calibration")
                Image {
                    id: rAe
                    anchors.centerIn: parent
                    width: parent.width
                    height: width
                    source: "qrc:/images/runAnalizer.png"
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
                    reciever.doMeasurements(graphs.series(seriesName), true);
                }
            }
            ToolButton {
                id: saveData
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
                                imgNameTF.text + ".png"
                        graphs.grabToImage(function(result) {
                            result.saveToFile(path);
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
                    graphs.zoomIn(Qt.rect(0, 0, graphs.width, graphs.height/2))
                }
            }
            ToolButton {
                id: zoomOut
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
                }
            }
            ToolButton {
                id: setSeriesVisible
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
                    for(var i = tableOfSeries.count-1; i>=0 ; i--) {
                        tableOfSeries.currentIndex = i;
                        if (tableOfSeries.currentItem.checked) {
                            graphs.series(tableOfSeries.currentItem.text
                                          ).visible = true;
                            console.log("Series: " +
                                        tableOfSeries.currentItem.text +
                                        " is off.");
                        } else {
                            graphs.series(tableOfSeries.currentItem.text
                                          ).visible = false;
                        }
                    }
                }
            }
            ToolButton {
                id: deleteSeries
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
                    }
                }
                onClicked: { messageDialog.setVisible(true) }
            }
        }
    }
}
