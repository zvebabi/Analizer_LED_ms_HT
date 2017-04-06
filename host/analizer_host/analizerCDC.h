#ifndef ANALIZERCDC_H
#define ANALIZERCDC_H

#include <QObject>
#include <QVariant>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <vector>

class analizerCDC : public QObject
{
    Q_OBJECT
public:
    explicit analizerCDC(QObject *parent = 0);

public slots:
    void cppSlot(const QString &msg);

    void initDevice(QString port);
    void getListOfPort();

//    void doMeasurements();
//    void doCalibration();
signals:
    void sendPortName(QString port);
private:
    std::vector<QString> ports;
    QSerialPort* device;
    QSerialPort::BaudRate baudRate;
//    int baudRate;

};

#endif // ANALIZERCDC_H
