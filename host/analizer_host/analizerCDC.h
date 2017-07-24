#ifndef ANALIZERCDC_H
#define ANALIZERCDC_H

#include <QtCore/QObject>
#include <QVariant>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>
#include <QByteArray>
#include <QPointF>
#include <QAbstractSeries>
#include <QtCharts/QAbstractSeries>
#include <QtCharts/QXYSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QDebug>
#include <QDir>
#include <vector>
#include <fstream>
#include <typeinfo>
#include <iostream>

#ifdef _WIN32 //winapi comport
#include "WinSerialPort.h"
#endif


class analizerCDC : public QObject
{
    Q_OBJECT
public:
    explicit analizerCDC(QObject *parent = 0);
    ~analizerCDC();
public slots:
    void cppSlot(const QString &msg);

    void initDevice(QString port);
    void getListOfPort();
    QString getDataPath() {return documentsPath;}
    void readData();
    void drawSeries(bool _draw) {drawLines = _draw;}
    void enableAAManual(bool _aa) {aaManual = _aa;}
//    void legendsFromMCU(bool _legend) {axisNameFromMCU = _legend;}
    void valuesFromMCU(bool _values) {axisValueFromMCU = _values;}

    void doMeasurements(QtCharts::QAbstractSeries *lineSeries,
                        QtCharts::QAbstractSeries *barSeries,
                        bool _etalon=false);
//    void doCalibration();
    void saveDataToCSV(QString filename);
    void deleteSeries(QtCharts::QAbstractSeries *series);

signals:
    void sendPortName(QString port);
    void sendDebugInfo(QString data);
    void adjustAxis(QPointF minRng, QPointF maxRng);
    void makeSeries();
private:
    void update(QtCharts::QAbstractSeries *series,
                QtCharts::QAbstractSeries *_barSeries);
    void processLine(const QByteArray& line);

    std::vector<QString> ports;
#ifdef _WIN32
    WinSerialPort* device = NULL;
#else
    QSerialPort* device = NULL;
#endif
    QSerialPort::BaudRate baudRate;
    QVector<QVector<QPointF> > m_data;
    bool etalon, drawLines, aaManual,axisNameFromMCU, axisValueFromMCU;
    bool isPortOpen, firstLine;
    QVector<QPointF> *currentPoints;
    QVector<QPointF> *etalonPoints;
    QtCharts::QAbstractSeries* currentSeries;
    QtCharts::QAbstractSeries* currentBarSeries;
    QtCharts::QAbstractSeries* etalonSeries;
    QMap< QtCharts::QAbstractSeries*, QVector<QPointF> > lines;
    QString documentsPath;
    float micrometers[42] = {2.3600, 2.3225, 2.2850, 2.2475,
                             2.2100, 2.1975, 2.1850, 2.1725,
                             2.1600, 2.1350, 2.1100, 2.0850,
                             2.0600, 2.0300, 2.0000, 1.9700,
                             1.9400, 1.9150, 1.8900, 1.8650,
                             1.8400,
                             1.7600, 1.7350, 1.7100, 1.685 ,
                             1.6600, 1.6325, 1.6050, 1.5775,
                             1.5500, 1.5400, 1.5300, 1.5200,
                             1.5100, 1.4950, 1.4800, 1.465,
                             1.4500, 1.4150, 1.3800, 1.3450,
                             1.3100 };
    QVector<QPointF> rangeVal; //min[0] max[1]
//    QVector<QtCharts::QAbstractSeries *series> saved_graphs;
//    int baudRate;
  int m_index;
};

#endif // ANALIZERCDC_H
