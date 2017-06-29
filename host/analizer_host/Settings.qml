import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtCharts 2.2
import "QMLs"

Item {
    Connections {
        target: reciever
        onSendPortName: {
            availablePorts.append({"text": port});
            console.log(port)
        }
        onSendDebugIngo: {
            tipsWithPath.showedText = qsTr("data is: " + data)
            tipsWithPath.open()
        }
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
            spacing: 10*app.dp
            anchors.right: chartStyle.left
            anchors.top: chartStyle.top
            anchors.rightMargin: 50*app.dp
            property int itemsWidth: 250*app.dp
            Button {
                id: listDeviceBTN
                contentItem: ButtonLabel {text: qsTr("ListDevice")}
                width: deviceSetter.itemsWidth
                onClicked: {//getPorts()
                    availablePorts.clear()
                    reciever.getListOfPort()
                }
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
                onClicked: reciever.initDevice(portsComboList.currentText)
            }
            CheckBox {
                id:drawSeries
                text: qsTr("Draw etalon data")
                checked: app.drawEt
                onClicked: {
                    reciever.drawSeries(checked)
                    app.drawEt = checked
                }
            }

        }
        Column {
            anchors.top: parent.top
            id: chartStyle
            TextField{
                readOnly: true
                text: qsTr("ChartStyle: ")
            }
            RadioButton {
                checked: true
                text: qsTr("Lines")
                onClicked: {
                    app.seriesType = ChartView.SeriesTypeLine
                }
            }
//            RadioButton {
//                text: qsTr("Histogam")
//                onClicked: {
//                    app.seriesType = ChartView.SeriesTypeBar
//                }
//            }
            RadioButton {
                text: qsTr("Spline")
                onClicked: {
                    app.seriesType = ChartView.SeriesTypeSpline
                }
            }
        }

    }
}
