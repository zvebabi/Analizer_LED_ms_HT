#pragma once
#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QPointF>
#include <QVector>
#include <QMap>

typedef struct DataSlot {
    QVector<QPointF> line;
    uint seq_number;
} DataSlot;

typedef enum MEASURE_MODE
{
    ABSOLUTE = 0,
    RELATIVE
} MeasureMode;

//Moveall data methods from AnalizerCDC
class DataHandler
{
public:
    DataHandler();

    //push data from device
    void PrepareToRecieveData(bool etalon_ = false);
    bool ProcessLineWithData(const QStringList& line, QString &ret);
    void setMode(int m) { m_mode = static_cast<MEASURE_MODE>(m); }
    void updateCalibrationParameters(QVector<double>& calibratorData) {
        calibrator_data = calibratorData; }

    void HideLine(uint n) {}
    void UnHideLine(uint n) {}

    void GetDataToShow() {}
    void RenameLine(const QString& oldName, const QString& newName) {}

private:
    bool dataAquisitionHandler(const QStringList& line);
    bool dataProcessingHandler(const QStringList& line, QString &ret);

    QMap<QString, DataSlot> m_data;
    std::vector<QString> m_names_to_show;
    DataSlot temp_slot;
    DataSlot etalon_slot;
    QVector<double> calibrator_data;
    bool current_slot_is_etalon;
    MEASURE_MODE m_mode; //or absolute
};

#endif // DATAHANDLER_H
