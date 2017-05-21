#ifndef ANALIZERCDC_H
#define ANALIZERCDC_H

#include <QtCore/QObject>
#include <QVariant>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>
#include <QByteArray>
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
    ~analizerCDC();
public slots:
    void cppSlot(const QString &msg);

    void initDevice(QString port, QString baudR);
    void getListOfPort();
    QString getDataPath() {return documentsPath;}
    void readData();

    void doMeasurements(QtCharts::QAbstractSeries *series);
//    void doCalibration();
    void saveDataToCSV(QString filename);
    void deleteSeries(QtCharts::QAbstractSeries *series);

signals:
    void sendPortName(QString port);
    void sendDebugInfo(QString data);
    void adjustAxis(QPointF minRng, QPointF maxRng);
    void makeSeries();
private:
    void update(QtCharts::QAbstractSeries *series);
    void processLine(const QByteArray& line);

    std::vector<QString> ports;
    QSerialPort* device = NULL;
    QSerialPort::BaudRate baudRate;
    QVector<QVector<QPointF> > m_data;
    QVector<QPointF> *currentPoints;
    QtCharts::QAbstractSeries* currentSeries;
    QMap< QtCharts::QAbstractSeries*, QVector<QPointF> > lines;
    QString documentsPath;
    QVector<QPointF> rangeVal; //min[0] max[1]
//    QVector<QtCharts::QAbstractSeries *series> saved_graphs;
//    int baudRate;
  int m_index;
};

#endif // ANALIZERCDC_H
