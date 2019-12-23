import QtQuick 2.8
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2
import QtQuick.Window 2.3
import QtGraphicalEffects 1.0
import Qt.labs.platform 1.0
//import QtCharts 2.2
import "QMLs"
import mydevice 1.0

ApplicationWindow {
    id:app
    property string appTitle: "LED Minispectrometer"
    property string appVersion: "1.24"
    property string chartType: "line"
//    property int seriesType: ChartView.SeriesTypeSpline
    property alias saveDataDialog_a: chV.editBar_a
    property alias relativeModeChBox_a: setV.check_box_relativeMeasurements_a
    visible: true
//    visibility: "FullScreen"
    flags: Qt.FramelessWindowHint | Qt.Window | Qt.WindowMinimizeButtonHint
    width: 1280 * dp
    height: 720 * dp
    minimumWidth: 1280 * dp
    minimumHeight: 720 * dp
    title: appTitle
    property bool drawEt: false //etalon draw or not
    property bool aaManual: false //Enable antialiasing
    property string yAxisName: qsTr("Absorbance") //Legends from mcu
    property bool valuesFromMCU: true //Values from mcu
    property bool serviceMode: false //write raw data to file
    property bool relativeMode: false   //compare with calibration data or with
                                        //previous sample
    property bool cumulativeMode: false
//    property bool ctmLegendVisibility: true
    Material.theme: Material.Light
    Material.primary: Material.BlueGrey
    Material.accent: Material.Teal

    property alias dp: device.dp
    MyDevice { id: device }

    QtObject {
            id: palette
            //http://www.materialpalette.com/indigo/yellow
            property color darkPrimary: "#303F9F"
            property color primary: "#3F51B5"
            property color lightPrimary: "#C5CAE9"
            property color text: "#FFFFFF"
            property color accent: "#FFEB3B"
            property color primaryText: "#212121"
            property color secondaryText: "#727272"
            property color divider: "#B6B6B6"

            property color currentHighlightItem: "#dcdcdc"
    }
//    property alias currentPage: loader.source
    property int menuWidth : 250*app.dp// width/4
    property int menuBarHeight: 50*app.dp
    property int widthOfSeizure: 15*app.dp
    property bool windowMaximized: false
    property bool dataWasChangedAfterSave: false
    property real menuProgressOpening
    property bool menuIsShown:
        Math.abs(menuView.x) < (menuWidth*0.5) ? true : false

      Rectangle {
      id: menuBar
      z: 5
      anchors.top: parent.top
      anchors.topMargin: 0
      width: parent.width
      height: app.menuBarHeight
      color:palette.darkPrimary // Material.color(Material.BlueGrey)
      Rectangle {
        id: menuButton
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: 1.2*height
        height: parent.height
        scale: maMenuBar.pressed ? 1.2 : 1
        color: "transparent"
        MenuIconLive {
          id: menuBackIcon
          scale: (parent.height/height)*0.65
          anchors.centerIn: parent
          value: menuProgressOpening
        }
        MouseArea {
          id: maMenuBar
          anchors.fill: parent
          onClicked: onMenu()
        }
        clip: true
      }
      Rectangle{
          id: windowTitleCustom
          anchors.left: menuButton.right
          anchors.right: quitButton.left
          anchors.verticalCenter: menuBar.verticalCenter
          height: parent.height
          color: "transparent"
          Label {
            id: titleText
            anchors.left: menuButton.right
            anchors.verticalCenter: parent.verticalCenter
            text: app.title
            font.pixelSize: 0.35*menuBar.height
            color: "#FFFFFF"
          }
          MouseArea {
              id: maWindowTitle
              anchors.fill: parent
              property variant clickPos: "1,1"
              onPressed: {
                  if ( ! app.windowMaximized )
                    clickPos  = Qt.point(mouse.x,mouse.y)
              }
              onPositionChanged: {
                  if ( ! app.windowMaximized )
                  {
                    var delta = Qt.point(mouse.x-clickPos.x, mouse.y-clickPos.y)
                    app.x += delta.x;
                    app.y += delta.y;
                  }
              }
              onDoubleClicked: {
                  console.log("doubleclicked")
                  if(app.windowMaximized )
                  {
                      app.showNormal()
                      app.width = app.minimumWidth
                      app.height = app.minimumHeight
                      app.windowMaximized =false
                  }
                  else
                  {
                      app.showMaximized()
                      app.windowMaximized =true
                  }
              }
          }
      }
      Rectangle {
          id: quitButton
          anchors.right: parent.right
          anchors.verticalCenter: parent.verticalCenter
          height: parent.height *0.8
          width: height
          scale: maMenuBarQuit.pressed ? 1.2 : 1
          color: "transparent"
          Image {
              id: quitBtnImg
              anchors.centerIn: parent
              width: parent.width
              height: width
              source: "qrc:/images/quit_48dp.png"
              antialiasing: true
              smooth: true
          }
          ColorOverlay {
              anchors.fill: quitBtnImg
              source: quitBtnImg
              color: "#ffffff"
          }
          MouseArea {
            id: maMenuBarQuit
            anchors.fill: parent
            onClicked:{
                app.dataWasChangedAfterSave ?
                    mainMenu.quitDialogCustom.setVisible(true) : Qt.quit()
            }
          }
      }
    } //menuBar

    Rectangle {
        id: menuView
        anchors.top: menuBar.bottom
        height: parent.height - menuBar.height
        width: menuWidth
        z: 3
        MainMenu {
            id: mainMenu
            anchors.fill: parent
            onMenuItemClicked: {
                onMenu()
//                loader.source = page
            }
        }
        x: -menuWidth

//        color: Material.color(Material.Grey)
        Behavior on x {
            NumberAnimation { duration: 500; easing.type: Easing.OutQuad } }
        onXChanged: {
            menuProgressOpening = (1-Math.abs(menuView.x/menuWidth))
        }

        MouseArea {
            anchors.right: parent.right
            anchors.rightMargin: app.menuIsShown ?
                                     (menuWidth - app.width) : -widthOfSeizure
            anchors.top: parent.top
            width: app.menuIsShown ? (app.width - menuWidth) : widthOfSeizure
            height: parent.height
            drag {
                target: menuView
                axis: Drag.XAxis
                minimumX: -menuView.width
                maximumX: 0
            }
            onClicked: {
                if(app.menuIsShown) app.onMenu()
            }
            onReleased: {
                if( Math.abs(menuView.x) > 0.5*menuWidth ) {
                    menuView.x = -menuWidth //close the menu
                } else {
                    menuView.x = 0 //fully opened
                }
            }
        }
    }
    Loader {
        id: loader
        anchors.top: menuBar.bottom;
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        //asynchronous: true
        onStatusChanged: {
            if( status == Loader.Loading ) {
                curtainLoading.visible = true
                titleText.text = appTitle
            } else if( status == Loader.Ready ) {
                curtainLoading.visible = false
            } else if( status == Loader.Error ) {
                curtainLoading.visible = false
            }
        }
        onLoaded: {
            titleText.text = item.title
        }
        Rectangle {
            id: curtainLoading
            anchors.fill: parent
            visible: false
            color: "white"
            opacity: 0.8
            BusyIndicator {
                anchors.centerIn: parent
            }
        }
    }
    function onMenu() {
        menuView.x = app.menuIsShown ? -menuWidth : 0
    }

    Component.onCompleted: {
//        currentPage = "Settings.qml"
        mainMenu.currentItem = 2
    }
    SwipeView {
        id: view
        interactive: false
        ///index fix because line and histigram viewer on same page,
        ///and nmenu numering is different.
        //indexes: 1 - line      - ChartView.qml
        //         2 - histogram - ChartView.qml
        //         0 - settings  - Settings.qml
        //         3 - about     - empty item
                currentIndex: mainMenu.currentItem < 2 ? 0 : mainMenu.currentItem - 1
        //        currentIndex: mainMenu.currentItem > 0 && mainMenu.currentItem < 3 ?
        //                          1 : mainMenu.currentItem == 0 ? 0 : mainMenu.currentItem - 1
        anchors.fill: parent
        anchors.top: menuBar.Bottom
        ChartView {
            id: chV
        }
        Settings {
            id: setV
        }
        About {
        }
    }
    Label {
        id: statusBar
        text: tipsWithPath.showedText
        color: "steelblue"
        anchors.bottom: parent.bottom
    }

    Popup {
        id: tipsWithPath
        x: app.width/2 - width/2
        y: app.height/2 - height/2
//        width: 200
//        height: 300
//        modal: true
        focus: true
        dim: true
        property string showedText: qsTr("")
        Text {
            id: textInPopup
            anchors.centerIn: tipsWithPath.Center
            text: tipsWithPath.showedText
        }
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    }
    onClosing: {
        mainMenu.quitDialogCustom.setVisible(true)
    }

}
