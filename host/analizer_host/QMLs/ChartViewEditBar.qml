import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtCharts 2.2
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0

Column {
    id: ctrlPane
    spacing: 3
    property int itemWidth: 300*app.dp
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
    //        Component {
    //            id: seriesDelegate
    //            Item {
    //                width: itemWidth; height: 50*app.dp
    //                Rectangle {
    //                    CheckBox {
    //                        id: checkBox
    ////                        height: 40
    ////                        width: height
    //                        onCheckStateChanged: {
    //                             tableModel.setProperty(index, "isChecked", !tableModel.get(index).isChecked)
    //                            console.log(tableModel.get(index).isChecked)
    //                        }
    //                    }
    //                    Text {
    //                        anchors.left: checkBox.right
    //                        anchors.verticalCenter: checkBox.verticalCenter
    //                        text: name }
    //                }
    //            }
    //        }

            ListView {
                anchors.fill: parent
                id: tableOfSeries
                model: ListModel { id: tableModel }
                delegate: CheckDelegate {
                    text: name
                    checked: isChecked
                    background: Rectangle {color: seriesColor}
                    width: ctrlPane.itemWidth - runAnalizer.width
                }//seriesDelegate
    //            highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
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
        }
        Grid {
            id : chartEditMenu
            columns: 1
            spacing: 6
            ToolButton {
                id: runAnalizer
                height: 48*app.dp
                width: height
                ToolTip.visible: hovered
                    ToolTip.text: qsTr("Run LED Minispectrometer")
                Image {
                    anchors.centerIn: parent
                    width: parent.width
                    height: width
                    source: "qrc:/images/runAnalizer.png"
                    antialiasing: true
                    smooth: true
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
                       "isChecked": false,
                       "seriesColor": graphs.series(seriesName).color.toString() })
                }
            }
            ToolButton {
                id: saveData
                height: 48*app.dp
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
                onClicked: reciever.saveDataToCSV(lineLabel.text);
            }
            ToolButton {
                id: saveImage
                height: 48*app.dp
                width: height
                ToolTip.visible: hovered
                    ToolTip.text: qsTr("Save image")
                Image {
                    anchors.fill: parent
                    source: "qrc:/images/takeSnapshot.png"
                    antialiasing: true
                    smooth: true
                }
                FileDialog {
                    id: imgSaveDialog
                    title: "Please choose a file"
                    selectExisting: false
                    nameFilters: [
                        "Image files (*.bmp *.jpg *.png)", "All files (*)" ]
                    folder: shortcuts.documents
                    onAccepted: {
                        var path = imgSaveDialog.fileUrl.toString();
                        path= path.replace(
                                   /^(file:\/{2})|(qrc:\/{2})|(http:\/{2})/,"");
                        // unescape html codes like '%23' for '#'
                        var cleanPath = decodeURIComponent(path);
                        console.log("image saved to " + cleanPath)
                        graphs.grabToImage(function(result) {
                            result.saveToFile(cleanPath);
                        });
                    }
                    onRejected: { console.log("Canceled") }
                }
                onClicked: {
                    imgSaveDialog.open();
                }
            }
            ToolButton {
                id: zoomIn
                height: 48*app.dp
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
                    graphs.zoomIn()
                }
            }
            ToolButton {
                id: zoomOut
                height: 48*app.dp
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
                    graphs.zoomOut()
                }
            }
            ToolButton {
                id: deleteSeries
                height: 48*app.dp
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
                                reciever.deleteSeries(graphs.series(tableOfSeries.currentItem.text));
                                graphs.removeSeries(graphs.series(tableOfSeries.currentItem.text));
                                tableModel.remove(i);
                                console.log("Series: " + tableOfSeries.currentItem.text+ " deleted.");
                            }
                        }
                    }
                }
                onClicked: {
                    messageDialog.setVisible(true)
                }
            }
        }
    }

//    Button {
//        text: "check"
//        onClicked: {
//            for(var i = 0; i< tableOfSeries.count; i++) {
//                tableOfSeries.currentIndex = i;
//               console.log( tableOfSeries.currentItem.text)
//            }
//        }
//    }

  /*
    Button {
        contentItem: ButtonLabel {text: qsTr("Analize")}
        width: ctrlPane.itemWidth
//                height :width/2.5
        onClicked: {
            graphs.numSeries++;
            var seriesName = qsTr(lineLabel.text + "_"
                                  + graphs.numSeries)
            graphs.createSeries(ChartView.SeriesTypeLine,
                                seriesName,
                                axisX, axisY);
            reciever.doMeasurements(graphs.series(seriesName));
            name4Del.text = seriesName
            allSeriesName.append(seriesName)
        }
    }
    TextField {
        id: name4Del
        selectByMouse: true
        width: ctrlPane.itemWidth
    }
    Button {
        width: ctrlPane.itemWidth
        contentItem: ButtonLabel {text: qsTr("Delete")}
        onClicked: {
            console.log(name4Del.text)
            reciever.deleteSeries(graphs.series(name4Del.text));
            graphs.removeSeries(graphs.series(name4Del.text));
        }
    }
    Button {
        width: ctrlPane.itemWidth
//                height :width/2.5
        contentItem: ButtonLabel {text: qsTr("Save data")}
        FileDialog {
            id: dataSaveDialog
            title: "Please choose a file"
            selectExisting: false
            nameFilters: [
                "Data files (*.csv *.dat *.txt)", "All files (*)" ]
            folder: shortcuts.documents
            onAccepted: {
                var path = dataSaveDialog.fileUrl.toString();
                path = path.replace(
                           /^(file:\/{2})|(qrc:\/{2})|(http:\/{2})/,"");
                // unescape html codes like '%23' for '#'
                var cleanPath = decodeURIComponent(path);
                console.log("You chose: " + cleanPath);
                reciever.saveDataToCSV(cleanPath);
            }
            onRejected: {
                console.log("Canceled")
            }
        }
        onClicked: {
//                    dataSaveDialog.open();
            reciever.saveDataToCSV(lineLabel.text);}
    }
    Button {
        width: ctrlPane.itemWidth
//                height :width/2.5
        contentItem: ButtonLabel {text: qsTr("Save graph")}

    }
*/
}
