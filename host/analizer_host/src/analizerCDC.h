#ifndef ANALIZERCDC_H
#define ANALIZERCDC_H

#include <QtCore/QObject>
#include <QVariant>
#include <QVector>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>
#include <QByteArray>
#include <QPointF>
#include <QtCharts/QAbstractSeries>
#include <QtCharts/QXYSeries>
#include <QColor>
#include <QVariantList>
#include <QStringList>
#include <QDebug>
#include <QDir>
#include <QPair>
#include <QFile>
#include <vector>
#include <fstream>
#include <typeinfo>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include "datahandler.h"

class AnalizerCDC : public QObject
{
    Q_OBJECT
public:
    explicit AnalizerCDC(QObject *parent = 0);
    ~AnalizerCDC();
public slots:
//    void cppSlot(const QString &msg);

    void initDevice(QString port);
    void getListOfPort();
    QString getDataPath() {return documentsPath;}
    void readData();
    void valuesFromMCU(bool _values) {axisValueFromMCU = _values;}
    void setServiceMode(bool _values) {serviceMode = _values;}
    void setRelativeMode(bool _values) {
        relativeMode = _values;
        dh.setMode(_values == true ? 1 : 0);
    }
    void setCumulativeMode(bool _values) {cumulativeMode = _values;}

    void doMeasurements(const QString seriesName);

//    void doCalibration();
    void selectPath(QString pathForSave);
    void saveDataToCSV(QString filename);
    void deleteSeries(const QString name);

    void readEtalonParameters(const QString filename, bool saveNew);

signals:
    void sendPortName(QString port);
    void sendDebugInfo(QString data, int time=700);
    void sendSerialNumber(QString serNumber);
    void sendEtalonName(QString etalonName);
    void adjustAxis(QPointF minRng, QPointF maxRng);
    void updateBarSeries(QString _label, QVariantList _data, QColor _color,
                         QStringList _axis);
    void disableButton();
    void activateEditBar();
    void activateRelativeMod();
    void deActivateRelativeMod();
    void makeSeries();
    void updateDrawer( QVector<QVector<QPointF>> data,
                       QVector<QString> legend,
                       QPointF minRng, QPointF maxRng
                      /*data and field borders*/); 
    void sendAxisName(QString data);
private:
    void update(); //call it after all changesin data
    void processLine(const QByteArray& line);
    void serviceModeHandler(const QStringList& line);
    void identityHandler(const QStringList& line);
    void buttonPressHandler(const QStringList& line);

    DataHandler dh;

    std::vector<QString> ports;
    QSerialPort* device = NULL;
    int m_serNumber;
    QSerialPort::BaudRate baudRate;
    bool etalon, axisNameFromMCU, axisValueFromMCU;
    bool isPortOpen, serviceMode , relativeMode, cumulativeMode;
    std::pair<QtCharts::QAbstractSeries*,QtCharts::QAbstractSeries*> currentSeries;
    QtCharts::QAbstractSeries* etalonSeries;
    QString documentsPath;
//    float micrometers[42] = {2.3600, 2.3225, 2.2850, 2.2475,
//                             2.2100, 2.1975, 2.1850, 2.1725,
//                             2.1600, 2.1350, 2.1100, 2.0850,
//                             2.0600, 2.0300, 2.0000, 1.9700,
//                             1.9400, 1.9150, 1.8900, 1.8650,
//                             1.8400,
//                             1.7600, 1.7350, 1.7100, 1.685 ,
//                             1.6600, 1.6325, 1.6050, 1.5775,
//                             1.5500, 1.5400, 1.5300, 1.5200,
//                             1.5100, 1.4950, 1.4800, 1.465,
//                             1.4500, 1.4150, 1.3800, 1.3450,
//                             1.3100 };
    QVector<QPointF> rangeVal; //min[0] max[1]
//    QVector<QtCharts::QAbstractSeries *series> saved_graphs;
//    int baudRate;
//    int m_index;
    std::ofstream diagnosticLog;

};

#endif // ANALIZERCDC_H
