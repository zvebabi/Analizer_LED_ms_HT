#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>

#include "analizerCDC.h"
#include "mydevice.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);

    QQuickStyle::setStyle("Material");
    qmlRegisterType<MyDevice>("mydevice", 1, 0, "MyDevice");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));

    QObject* root = engine.rootObjects()[0];

    analizerCDC *analizer= new analizerCDC(root);

    engine.rootContext()->setContextProperty("reciever", analizer);

//    QObject::connect(root, SIGNAL(getPorts()),
//                     analizer, SLOT(getListOfPort()));
//    QObject::connect(root, SIGNAL(initDevice(QString, QString)),
//                     analizer, SLOT(initDevice(QString, QString)));
//    QObject::connect(root, SIGNAL(doMeasurements()),
//                     analizer, SLOT(doMeasurements()));

    return app.exec();
}
