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
#include <QDir>
#include <vector>
#include <fstream>
#include <typeinfo>
#include <iostream>

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
    void adjustAxis(QPointF minRng, QPointF maxRng);
private:
    void update(QtCharts::QAbstractSeries *series);


    std::vector<QString> ports;
    QSerialPort* device;
    QSerialPort::BaudRate baudRate;
    QList<QVector<QPointF> > m_data;
    QMap< QtCharts::QAbstractSeries*, QVector<QPointF> > lines;
    QString documentsPath;
    QVector<QPointF> rangeVal; //min[0] max[1]
//    QVector<QtCharts::QAbstractSeries *series> saved_graphs;
//    int baudRate;
  int m_index;
};

#endif // ANALIZERCDC_H
