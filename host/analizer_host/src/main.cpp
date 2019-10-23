#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>
#include <QIcon>
#include "analizerCDC.h"
#include "plugins/ScreenProperties/screenproperties.h"
#include "plugins/FileValidator/filevalidator.h"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    qApp->setQuitOnLastWindowClosed(true);
    QQuickStyle::setStyle("Material");
    qmlRegisterType<ScreenProperties>("mydevice", 1, 0, "MyDevice");
    qmlRegisterType<FileValidator>("filevalidator", 1, 0, "FileValidator");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));

    QObject* root = engine.rootObjects()[0];
    app.setWindowIcon(QIcon(":/images/logo_icon_7.png"));
    AnalizerCDC *analizer= new AnalizerCDC(root);

    engine.rootContext()->setContextProperty("reciever", analizer);

//    QObject::connect(root, SIGNAL(getPorts()),
//                     analizer, SLOT(getListOfPort()));
//    QObject::connect(root, SIGNAL(initDevice(QString, QString)),
//                     analizer, SLOT(initDevice(QString, QString)));
//    QObject::connect(root, SIGNAL(doMeasurements()),
//                     analizer, SLOT(doMeasurements()));

    return app.exec();
}
