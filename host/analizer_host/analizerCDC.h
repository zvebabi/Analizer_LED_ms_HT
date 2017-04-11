#ifndef ANALIZERCDC_H
#define ANALIZERCDC_H

#include <QtCore/QObject>
#include <QVariant>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QAbstractSeries>
#include <QtCharts/QAbstractSeries>
#include <QtCharts/QXYSeries>
#include <QDebug>
#include <vector>
#include <fstream>
#include <typeinfo>

class analizerCDC : public QObject
{
    Q_OBJECT
public:
    explicit analizerCDC(QObject *parent = 0);

public slots:
    void cppSlot(const QString &msg);

    void initDevice(QString port, QString baudR);
    void getListOfPort();

    void doMeasurements(QtCharts::QAbstractSeries *series);
//    void doCalibration();
    void saveDataToCSV(QString filename);
    void deleteSeries(QtCharts::QAbstractSeries *series);
signals:
    void sendPortName(QString port);
private:
    void update(QtCharts::QAbstractSeries *series);


    std::vector<QString> ports;
    QSerialPort* device;
    QSerialPort::BaudRate baudRate;
    QList<QVector<QPointF> > m_data;
    QMap< QtCharts::QAbstractSeries*, QVector<QPointF> > lines;
//    QVector<QtCharts::QAbstractSeries *series> saved_graphs;
//    int baudRate;
  int m_index;
};

#endif // ANALIZERCDC_H