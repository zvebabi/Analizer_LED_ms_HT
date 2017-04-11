import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtCharts 2.2
import "QMLs"

Item {
    RowLayout {
        spacing: 10
        anchors.fill: parent
        anchors.margins: 30


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
            spacing: 10
            TextField {
                id: lineLabel
                selectByMouse: true
                width: 100
            }
            Button {
                contentItem: ButtonLabel {text: qsTr("Analize")}
                width: 100
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
                width: 100
            }
            Button {
                width: 100
                contentItem: ButtonLabel {text: qsTr("Delete")}
                onClicked: {
                    console.log(name4Del.text)
                    reciever.deleteSeries(graphs.series(name4Del.text));
                    graphs.removeSeries(graphs.series(name4Del.text));
                }
            }
            Button {
                width: 100
                contentItem: ButtonLabel {text: qsTr("Save data")}
                onClicked: {
                    reciever.saveDataToCSV("data.csv");
                }
            }
        }
    }
}

