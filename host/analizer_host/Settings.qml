import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtCharts 2.2
import QtQuick.Dialogs 1.2
import "QMLs"

Item {
    Connections {
        target: reciever
        onSendPortName: {
            availablePorts.append({"text": port});
            console.log(port)
        }
        onSendDebugInfo: {
            tipsWithPath.showedText = qsTr(data)
            tipsWithPath.open()
            delay(time, tipsWithPath.close)
        }
        onSendAxisName: {
            app.yAxisName = qsTr(data)
        }
        onDisableButton: {
            connectBTN.enabled = false
            listDeviceBTN.enabled = false
            portsComboList.enabled =false
        }
        onActivateRelativeMod: {
            relativeMeasurements.checked = true
            app.relativeMode = true
            reciever.setRelativeMode(true)
        }
    }
    Timer {
        id: timer
    }
    function delay(delayTime, cb) {
        timer.interval = delayTime;
        timer.repeat = false;
        timer.triggered.connect(cb);
        timer.start();
    }
    Grid{
        columns: 2
        spacing: 50*app.dp
//        anchors.fill: parent
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 100*app.dp
        anchors.top: parent.top
        Column {
            id: deviceSetter
            spacing: 5*app.dp
//            anchors.right: chartStyle.left
//            anchors.top: chartStyle.top
            anchors.rightMargin: 50*app.dp
            property int itemsWidth: 250*app.dp
            Button {
                id: listDeviceBTN
                contentItem: ButtonLabel {text: qsTr("Select Device")}
                width: deviceSetter.itemsWidth
                onClicked: {//getPorts()
                    availablePorts.clear()
                    reciever.getListOfPort()
                }
//                height: 150*app.dp
            }

            ComboBox{
                id: portsComboList
                objectName: "comboList"
                model: availablePorts
                width: deviceSetter.itemsWidth
                ListModel{
                    id: availablePorts
                }

            }

//            ComboBox {
//                id: baudRateComboList
//                objectName: "baudRate"
//                width: deviceSetter.itemsWidth
//                model: ["115200", "57600"]
//            }

            Button {
                id: connectBTN
                contentItem: ButtonLabel {text: qsTr("Connect")}
                width: deviceSetter.itemsWidth
                onClicked: {
                    reciever.initDevice(portsComboList.currentText);
//                    app.ctmLegendVisibility = false;
                }
            }
            Label {
                text: qsTr("Save images and data to: ")
                font.family: "DejaVu Sans Mono"
                font.pixelSize: 22*app.dp
            }
            TextField {
                id: filePathText
                width: deviceSetter.itemsWidth
                text:reciever.getDataPath()
                font.family: "DejaVu Sans Mono"
                font.pixelSize: 22*app.dp
                readOnly: true
                selectByMouse: true
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onClicked: {
                        if(mouse.button === Qt.RightButton) {
                            filePathText.copy()
                            tipsWithPath.showedText = qsTr("Path has been copied to clipboard")
                            tipsWithPath.open()
                            delay(1500, tipsWithPath.close)
                            filePathText.deselect()
                        }
                        if(mouse.button === Qt.LeftButton) {
                            filePathText.selectAll()
                        }
                    }
                }
            }
            Button
            {
                id: selectPath
                contentItem:  ButtonLabel {text:qsTr("Change path")}
                width: deviceSetter.itemsWidth
                FileDialog {
                    id: fileDialog
                    title: qsTr("Select directory")
                    visible: false
                    folder: "file:///" + reciever.getDataPath()
                    selectExisting: true
                    selectFolder: true
                    selectMultiple: false
                    onAccepted: {
                        reciever.selectPath(fileUrl.toString().substring(8) + "/")
                        filePathText.text = reciever.getDataPath()
                    }
                }
                onClicked: fileDialog.open()
            }

            CheckBox {
                id:relativeMeasurements
                text: qsTr("Relative mode")
                checked: app.relativeMode
                onClicked: {
                    reciever.setRelativeMode(checked)
                    app.relativeMode = checked
                }
            }
//            CheckBox {
//                id:drawSeries
//                text: qsTr("Draw etalon data")
//                checked: app.drawEt
//                onClicked: {
//                    reciever.drawSeries(checked)
//                    app.drawEt = checked
//                }
//            }

//            CheckBox {
//                id:serviceMode
//                text: qsTr("Save raw data")
//                checked: app.serviceMode
//                onClicked: {
//                    reciever.setServiceMode(checked)
//                    app.serviceMode = checked
//                }
//            }
//            CheckBox {
//                id:cumulativeMeasurements
//                text: qsTr("Cumulative mode")
//                checked: app.cumulativeMode
//                onClicked: {
//                    reciever.setCumulativeMode(checked)
//                    app.cumulativeMode = checked
//                }
//            }
//            CheckBox {
//                id:antialiasingManual
//                text: qsTr("Enable antialiasing")
//                checked: app.aaManual
//                onClicked: {
//                    reciever.enableAAManual(checked)
//                    app.aaManual = checked
//                }
//            }
//            RadioButton {
//                id:name1
//                checked: true
//                text: qsTr("Absorbance")
//                onClicked: {
//                    app.yAxisName = name1.text
//                }
//            }
//            RadioButton {
//                id:name2
//                checked: false
//                text: qsTr("Transmittance")
//                onClicked: {
//                    app.yAxisName = name2.text
//                }
//            }
        }

//        //implement hystogram/lines
//        Column {
//            anchors.top: parent.top
//            id: chartStyle
//            TextField{
//                readOnly: true
//                text: qsTr("ChartStyle: ")
//            }

//            RadioButton {
//                checked: true
//                text: qsTr("Graph view")
//                onClicked: {
////                    app.seriesType = ChartView.SeriesTypeSpline
//                }
//            }
//        }

    }
}
