#include "analizerCDC.h"
#include <QtMath>
analizerCDC::analizerCDC(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QtCharts::QAbstractSeries*>();
    qRegisterMetaType<QtCharts::QAbstractAxis*>();
    documentsPath = QDir::currentPath()+QString("/data/");
    rangeVal.append(QPointF(0,0));
    rangeVal.append(QPointF(0,0));
}

void analizerCDC::cppSlot(const QString &msg)
{
    //Найдем строки ввода
    QObject* field1 = this->parent()->findChild<QObject*>("field1");
    QObject* field2 = this->parent()->findChild<QObject*>("field2");
    //Найдем поле вывода
    QObject* textArea = this->parent()->findChild<QObject*>("textArea");

    //Считаем информацию со строк ввода через свойство text
    QString str1=(field1->property("text")).toString();
    QString str2=(field2->property("text")).toString();

    int a = str1.toInt();//Переведем строку в число
    int b = str2.toInt();//Переведем строку в число

    int c = a + b; //Вычисления наши

    QString strResult=QString::number(c);//Переведем результат в строку

    //Ну и наконец выведем в поле вывода нашу информацию
    textArea->setProperty("text", str1+" + "+str2+" = "+strResult+" "+msg);
}

void analizerCDC::initDevice(QString port, QString baudR)
{
#if 1
    qDebug() << port << "\n" << baudR;
#else
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        if (info.portName() == port) {
            device = new QSerialPort(info);
            break;
        }
    }
//    QObject::connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
    if(!(device->isOpen()))
        device->open(QIODevice::ReadWrite);
    device->setBaudRate(QSerialPort::Baud115200);
    device->setDataBits(QSerialPort::Data8);
    device->setParity(QSerialPort::NoParity);
    device->setStopBits(QSerialPort::OneStop);
    device->setFlowControl(QSerialPort::NoFlowControl);
#endif
}

void analizerCDC::getListOfPort()
{
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        ports.push_back(info.portName());
//        emit sendPortName(info.portName());
        qDebug() << info.portName();
    }
    qDebug() << "dfd";
    emit sendPortName(QString("teststr"));
//    QObject* comboBox = this->parent()->findChild<QObject*>("availablePorts");
    //    comboBox->setProperty("append", info.portName());
}

void analizerCDC::doMeasurements(QtCharts::QAbstractSeries *series)
{
    qDebug() << "doMeasurements";
    auto colCount =100;
    // Append the new data depending on the type
    QVector<QPointF> points;
    points.reserve(colCount);
    for (int j(0); j < colCount; j++) {
        qreal x(0);
        qreal y(0);
      // data with sin + random component
        y = qSin(3.14159265358979 / 30 * j) + 0.5 + (qreal) rand() / (qreal) RAND_MAX;
        x = j;
        //find borders
        if ( rangeVal[0].x() > x )
            rangeVal[0].setX(x);
        if ( rangeVal[1].x() < x )
            rangeVal[1].setX(x);
        if ( rangeVal[0].y() > y )
            rangeVal[0].setY(y);
        if ( rangeVal[1].y() < y )
            rangeVal[1].setY(y);
        //add data to graph
        points.append(QPointF(x, y));
    }
    m_data.append(points);
    lines.insert(series, m_data.back()); //save series and data pointers for future
    emit adjustAxis(rangeVal[0], rangeVal[1]);
    update(series);
}

void analizerCDC::saveDataToCSV(QString filename="data.csv")
{
    qDebug() << "save to csv..";
    qDebug() <<filename;
    qDebug() << documentsPath;

    QDir dataDir(documentsPath);
    if (!dataDir.exists())
        dataDir.mkpath(".");

    std::fstream f(QString(documentsPath+"/"+filename+".csv").toStdString(), std::fstream::out);
    if (!f.is_open())
        qDebug() << "can't open file\n";
    int i = 1;
    for (auto series : lines.keys())
    {
        f << "Sample: " << series->name().toStdString() << "\n";
        for ( auto& p : lines.value(series))
            f << p.x() << ", " << p.y() << "\n";
        ++i;
    }
    f.close();
    qDebug() << ".done";
}

void analizerCDC::deleteSeries(QtCharts::QAbstractSeries *series)
{
    if (lines.contains(series))
        lines.remove(series);
}

void analizerCDC::update(QtCharts::QAbstractSeries *series)
{
    if (series && lines.contains(series)) {
        QtCharts::QXYSeries *xySeries = static_cast<QtCharts::QXYSeries *>(series);
//        QVector<QPointF> points = lines.value(series);
        // Use replace instead of clear + append, it's optimized for performance
        xySeries->replace(lines.value(series));
    }
}


















