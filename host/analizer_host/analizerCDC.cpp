#include "analizerCDC.h"
#include <QtMath>
analizerCDC::analizerCDC(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QtCharts::QAbstractSeries*>();
    qRegisterMetaType<QtCharts::QAbstractAxis*>();
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
    device->setBaudRate(QSerialPort::Baud57600);
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
    // Remove previous data
//    foreach (QVector<QPointF> row, m_data)
//        row.clear();
//    m_data.clear();
    auto colCount =100;
    // Append the new data depending on the type
    QVector<QPointF> points;
    points.reserve(colCount);
    for (int j(0); j < colCount; j++) {
        qreal x(0);
        qreal y(0);
      // data with sin + random component
        y = qSin(3.14159265358979 / 50 * j) + 0.5 + (qreal) rand() / (qreal) RAND_MAX;
        x = j;
        points.append(QPointF(x, y));
    }
    m_data.append(points);
    lines.insert(series, m_data.back()); //save series and data pointers for future
    update(series);
}

void analizerCDC::saveDataToCSV(QString filename="data.csv")
{
    qDebug() << "save to csv..";
    std::fstream f(filename.toStdString(), std::fstream::out);
    int i = 1;
    for (auto series : lines.keys())
    {
//        f << "sample N " << i << "\n";
        f << "Sample: " << series->name().toStdString() << "\n";
        for ( auto& p : lines.value(series))
        {
            f << p.x() << ", " << p.y() << "\n";
        }
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
    if (series) {
        QtCharts::QXYSeries *xySeries = static_cast<QtCharts::QXYSeries *>(series);
        m_index++;
        if (m_index > m_data.count() - 1)
            m_index = 0;

        QVector<QPointF> points = m_data.at(m_index);
        // Use replace instead of clear + append, it's optimized for performance
        xySeries->replace(points);
    }
}


















