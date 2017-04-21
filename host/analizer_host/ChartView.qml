import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtCharts 2.2
import QtQuick.Dialogs 1.2
import "QMLs"

Item {
    RowLayout {
        spacing: 1
        anchors.fill: parent
        anchors.margins: 10
        ChartView {
            id: graphs
            title: "Result"
            antialiasing: true
            property int numSeries : 0 //current number of graphs
            ValueAxis {
                id: axisX
                min: 0
                max: 100
                tickCount: 5
            }
            ValueAxis {
                id: axisY
                min: -0.5
                max: 2
            }
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: 320
            Layout.minimumHeight: 240
        }
        Column {
            id :ctrlPane
            anchors.top: menuBar.bottom
            spacing: 5
            property int itemWidth: 80
            TextField {
                id: lineLabel
                selectByMouse: true
                width: ctrlPane.itemWidth
            }
            Button {
                contentItem: ButtonLabel {text: qsTr("Analize")}
                width: ctrlPane.itemWidth
                height :width/2.5
                onClicked: {
                    graphs.numSeries++;
                    graphs.createSeries(ChartView.SeriesTypeLine,
                                        lineLabel.text + graphs.numSeries,
                                        axisX, axisY);
                    reciever.doMeasurements(graphs.series(lineLabel.text
                                                          + graphs.numSeries));
                }
            }
            TextField {
                id: name4Del
                selectByMouse: true
                width: ctrlPane.itemWidth
                height :width/2.5
            }
            Button {
                width: ctrlPane.itemWidth
                height :width/2.5
                contentItem: ButtonLabel {text: qsTr("Delete")}
                onClicked: {
                    console.log(name4Del.text)
                    reciever.deleteSeries(graphs.series(name4Del.text));
                    graphs.removeSeries(graphs.series(name4Del.text));
                }
            }
            Button {
                width: ctrlPane.itemWidth
                height :width/2.5
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
                height :width/2.5
                contentItem: ButtonLabel {text: qsTr("Save graph")}
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
        }
    }

}

