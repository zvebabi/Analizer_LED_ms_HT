import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtCharts 2.3
import QtQuick.Dialogs 1.2
import "QMLs"

Item {
    property alias check_box_relativeMeasurements_a : relativeMeasurements
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
            relativeMeasurements.enabled = false
            etalonNameLbl.text = qsTr("Load etalon data file to switch"
                                    + " to Absolute measuring mode")
            app.relativeMode = true
            reciever.setRelativeMode(true)
            selectEtalonPath.visible = true
        }
        onDeActivateRelativeMod: {
            relativeMeasurements.checked = false
            relativeMeasurements.enabled = true
            app.relativeMode = false
            reciever.setRelativeMode(false)
            selectEtalonPath.visible = false
//            selectEtalonPath.background = "green"
        }
        onSendSerialNumber: {
            serNumLbl.text = serNumber;
        }
        onSendEtalonName: {
            etalonNameLbl.text = etalonName;
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
            anchors.horizontalCenter: parent.horizontalCenter
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

            ComboBox {
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
                visible: false
            }
            TextField {
                id: filePathText
                visible: false
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
            Button {
                id: selectPath
                contentItem:  ButtonLabel {text:qsTr("Save data to…")}
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
            Button {
                id: selectEtalonPath
                contentItem:  ButtonLabel {text:qsTr("Etalon data")}
                width: deviceSetter.itemsWidth
                visible: false
                FileDialog {
                    id: fileDialogCalibr
                    title: qsTr("Select file with etalon data")
                    visible: false
                    folder: "file:///" + reciever.getDataPath()
                    selectExisting: true
//                    selectFolder: false
                    selectMultiple: false
                    onAccepted: {
                        console.log(fileUrl);
                        reciever.readEtalonParameters(fileUrl.toString().substring(8), true)
                    }
                }
                onClicked: fileDialogCalibr.open()
            }
            CheckBox {
                id:relativeMeasurements
                enabled: true
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
    Column {
        id: currentDeviceSetting
//            anchors.top: parent.top
        spacing: 5*app.dp
//        anchors.left: 100*app.dp
//        y: app.height - height -statusBar.height - 10*app.dp
        anchors.bottom: parent.bottom
        anchors.bottomMargin: statusBar.height + 10*app.dp
        anchors.left: parent.left
        anchors.leftMargin: anchors.bottomMargin
//            property int itemsWidth: 250*app.dp
        Grid {
            columns: 2
            Label { text: qsTr("Device serial number: ") }
            Label { id:serNumLbl; text: "Not connected" }

            Label { text: qsTr("Etalon : ") }
            Label { id: etalonNameLbl; text: qsTr("Not found") }

            Label { text: qsTr("Сalibration mode: ") }
            Label { id: calModeLbl
                    text: app.relativeMode ? qsTr("Relative") : qsTr("Absolute") }

            Label { text: qsTr("Measurement data storage directory: ") }
            Label { id: pathLbl; text: filePathText.text }
        }
        Label { text: qsTr("Click \"Save data to…\" to choose another location") }
    }
}
