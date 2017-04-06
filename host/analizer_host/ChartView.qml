import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

Item {
    Column {
        spacing: 10
        anchors.centerIn: parent

        TextField {
            id: field1
            objectName: "field1"
            placeholderText: "Введите первое число"
            width: 250
        }

        TextField {
            id: field2
            objectName: "field2"
            placeholderText: "Введите второе число"
            width: 250
        }

        Button {
            id: button
            text: qsTr("Сложить")
            width: 250
            onClicked: qmlSignal("яблок")
        }

        Text {
            text: "Ответ:"
            font.pixelSize: textArea.font.pixelSize
        }

        TextArea {
            id: textArea
            objectName: "textArea"
            wrapMode: TextArea.Wrap
            readOnly: true
            width: 250
        }
    }
}
