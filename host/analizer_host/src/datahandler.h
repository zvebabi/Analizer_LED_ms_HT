#pragma once
#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QPointF>
#include <QVector>
#include <QMap>

typedef struct DataSlot
{
    QString name;
    QVector<QPointF> line;
} DataSlot;

typedef enum MEASURE_MODE
{
    ABSOLUTE = 0,
    RELATIVE
} MEASURE_MODE;

//Moveall data methods from AnalizerCDC
class DataHandler
{
public:
    DataHandler();

    //push data from device
    void PrepareToRecieveData(QString line_name, bool etalon_ = false);
    bool ProcessLineWithData(const QStringList& line, QString &ret);

    void setMode(int m) { m_mode = static_cast<MEASURE_MODE>(m); }

    void updateCalibrationParameters(QVector<double>& calibratorData) {
        calibrator_data = calibratorData; 
    }
    //data editors
    void HideLine(const QString name_) {
        if( m_data.count(name_) != 0 && m_names_to_show.count(name_) != 0 )
            m_names_to_show.removeAll(name_);
    }
    void UnHideLine(const QString name_) {
        if( m_data.count(name_) != 0 && m_names_to_show.count(name_) == 0 )
            m_names_to_show.push_back(name_);
    }
    //data modifiers
    bool DeleteLine(const QString name_);
    bool RenameLine(const QString& oldName, const QString& newName);

    uint GetDataToShow(QVector<DataSlot>& data);
    uint GetAllData(QVector<DataSlot>& data);

private:
    bool dataAquisitionHandler(const QStringList& line, QString& ret);
    bool dataProcessingHandler(const QStringList& line, QString& ret);

    QMap<QString, DataSlot> m_data;
    QVector<QString> m_names_refs; //use to preserve order
    QVector<QString> m_names_to_show; //use to send to gui actual lines
    QString temp_slot_name;
    DataSlot temp_slot;
    DataSlot etalon_slot;
    QVector<double> calibrator_data; //readed from file
    bool current_slot_is_etalon;
    MEASURE_MODE m_mode; //or absolute
};

#endif // DATAHANDLER_H
