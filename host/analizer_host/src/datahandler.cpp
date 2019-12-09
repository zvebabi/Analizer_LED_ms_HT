#include "datahandler.h"
#include <cmath>

#include <QDebug>

template<class T>
bool nearlyEqual(T a, T b, T epsilon = 0.005) {
    return fabs(a - b) <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

DataHandler::DataHandler() : current_slot_is_etalon(false)
{

}

void DataHandler::PrepareToRecieveData(QString line_name, bool etalon_)
{
    current_slot_is_etalon = etalon_;
    temp_slot_name = line_name;
    
    temp_slot.line.clear();
    if ( !current_slot_is_etalon ) {
        m_data[line_name] = DataSlot();
        m_data[line_name].name = line_name;
        m_names_to_show.push_back(line_name);
        m_names_refs.push_back(line_name);
    }
}

bool DataHandler::ProcessLineWithData(const QStringList &line, QString& ret)
{
    if ( line.first().compare("x=d") == 0 ) {
        return dataAquisitionHandler(line, ret);
    } else if ( line.first().compare("x=e\n") == 0 ) {
        return dataProcessingHandler(line, ret);
    }
    ret = "broken data";
    return false;
}

void DataHandler::HideLine(const QString& name_)
{
    if( m_data.count(name_) != 0 && m_names_to_show.count(name_) != 0 )
        m_names_to_show.removeAll(name_);
    recalculateDataRange();
}

void DataHandler::UnHideLine(const QString& name_)
{
    if( m_data.count(name_) != 0 && m_names_to_show.count(name_) == 0 )
        m_names_to_show.push_back(name_);
    recalculateDataRange();
}

bool DataHandler::dataAquisitionHandler(const QStringList &line, QString& ret)
{
    if ( line.size() == 4 ) {
        auto x = line.at(1).toDouble();
        auto y = line.at(3).toDouble();
        temp_slot.line.append(QPointF(x, y));
        ret = "point ok";
        return true;
    } else {
        ret = "bad point";
        return false;
    }
}

/*!
 * @param line Input data
 * @param ret  Field to return status
 */
bool DataHandler::dataProcessingHandler(const QStringList &line, QString& ret)
{
    for (auto point = temp_slot.line.begin(); point != temp_slot.line.end(); point++)
    {
        if (nearlyEqual(point->y(), -1.0))
        {
            ret = "Weak signal! Redo measurement!";
            return false;
        }
    }
    //save series and data for future
    if (current_slot_is_etalon)
    {
        etalon_slot.line.clear();

        for (int i=0; i < temp_slot.line.size(); i++)
        {
            auto xVal = temp_slot.line.at(i).x();
            auto yVal = m_mode == MEASURE_MODE::RELATIVE ?
                            temp_slot.line.at(i).y() :
                            temp_slot.line.at(i).y() / calibrator_data[i];
            etalon_slot.line.append(QPointF( xVal, yVal ) );
        }
        qDebug() << "set etalon";
        ret = "activateEditBar";
    } else {
        //calibrate
        for (int i=0; i < temp_slot.line.size(); i++)
        {
            m_data[temp_slot_name].line.append(
                QPointF(temp_slot.line.at(i).x(),
                 temp_slot.line.at(i).y() / etalon_slot.line.at(i).y()*100.0));
        }

        //TODO move to drawer
        /*with part adjust roi borders to fit in graph field
        qreal xMin = std::numeric_limits<qreal>::max(); // everything is <= this
        qreal xMax = std::numeric_limits<qreal>::min(); // everything is >= this
        qreal yMin = std::numeric_limits<qreal>::max();
        qreal yMax = std::numeric_limits<qreal>::min();*/
        for(const auto& p : m_data[temp_slot_name].line) {
            bl.setX( qMin(bl.x(), p.x()) );
            bl.setY( qMin(bl.y(), p.y()) );
            tr.setX( qMax(tr.x(), p.x()) );
            tr.setY( qMax(tr.y(), p.y()) );
        }
        qDebug() << "bl: " << bl << " tr: " << tr;
        /*
        if(firstLine)
        {
            firstLine = false;
            rangeVal[0].setX(xMin);
            rangeVal[1].setX(xMax);
            rangeVal[0].setY(yMin);
            rangeVal[1].setY(yMax);
        }
        //find borders
        if ( rangeVal[0].x() > xMin )
            rangeVal[0].setX(xMin);
        if ( rangeVal[1].x() < xMax )
            rangeVal[1].setX(xMax);
        if ( rangeVal[0].y() > yMin )
            rangeVal[0].setY(yMin);
        if ( rangeVal[1].y() < yMax )
            rangeVal[1].setY(yMax);
        */

        ret = "updateDrawer";
    }
    return true;
}

void DataHandler::recalculateDataRange()
{
    QPointF new_bl = {std::numeric_limits<qreal>::max(),std::numeric_limits<qreal>::max()}; //range of 2D data
    QPointF new_tr = {std::numeric_limits<qreal>::min(),std::numeric_limits<qreal>::min()}; //
    for (auto& name_ : m_names_to_show) {
        for(const auto& p : m_data[name_].line) {
            new_bl.setX( qMin(bl.x(), p.x()) );
            new_bl.setY( qMin(bl.y(), p.y()) );
            new_tr.setX( qMax(tr.x(), p.x()) );
            new_tr.setY( qMax(tr.y(), p.y()) );
        }
    }
    bl = new_bl;
    tr = new_tr;
}

bool DataHandler::DeleteLine(const QString &name_) {
    HideLine(name_);
    if( m_data.count(name_) != 0 ) {
        m_names_refs.removeAll(name_);
        m_data.remove(name_);
    } else {
        return false;
    }
    return true;
}

bool DataHandler::RenameLine(const QString& oldName, const QString& newName) {
    //re-add data in Map
    auto dataToRename = m_data.find(oldName);
    if(  dataToRename == m_data.end() ) {
        qDebug() << "Line with name " << oldName << " not found!";
        return false;
    }
    //TODO look for key renaming api
    m_data[newName] = *dataToRename;
    m_data[newName].name = newName;
    m_data.remove(oldName);
    //edit refnames
    //TODO refactor it with QVector API
    for (auto& n : m_names_refs) {
        if ( n == oldName)
            n = newName;
    }
    //if line is showed - rename it too 
    if( m_names_to_show.count(oldName) != 0 ) {
        for (auto& n : m_names_to_show) {
            if ( n == oldName)
                n = newName;
        }
    }
    return true;
}
    
std::tuple<QStringList, QPointF, QPointF> DataHandler::GetSeriesNamesToShow()
{
    QStringList legends;

    for(const auto& name : m_names_to_show) {
        legends.push_back(m_data[name].name);
    }
    return {legends, bl, tr} ;
}

uint DataHandler::GetAllData(QVector<DataSlot>& data)
{
    data.clear();
    for(auto& name : m_names_refs)
        data.append(m_data[name]);

    return uint(data.size());
}

QVector<QPointF> DataHandler::GetLineByName(const QString &name_)
{
    if ( m_names_to_show.count(name_) ) {
        return m_data[name_].line;
    } else {
        return {};
    }
}
