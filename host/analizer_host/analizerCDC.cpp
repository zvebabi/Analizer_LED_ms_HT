#include "analizerCDC.h"

analizerCDC::analizerCDC(QObject *parent) : QObject(parent)
{

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

void analizerCDC::initDevice(QString port)
{
#if 1
    qDebug() << port;
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




















