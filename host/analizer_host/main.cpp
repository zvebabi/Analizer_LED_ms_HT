#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>

#include "analizerCDC.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QQuickStyle::setStyle("Material");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));

    QObject* root = engine.rootObjects()[0];

    analizerCDC *analizer= new analizerCDC(root);

    QQmlContext* ctx = engine.rootContext();
    ctx->setContextProperty("reciever", analizer);

    QObject::connect(root, SIGNAL(qmlSignal(QString)),
                     analizer, SLOT(cppSlot(QString)));
    QObject::connect(root, SIGNAL(getPorts()),
                     analizer, SLOT(getListOfPort()));
    QObject::connect(root, SIGNAL(initDevice(QString)),
                     analizer, SLOT(initDevice(QString)));

    return app.exec();
}
