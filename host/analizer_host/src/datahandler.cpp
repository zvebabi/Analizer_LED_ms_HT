#include "datahandler.h"
#include <cmath>

template<class T>
bool nearlyEqual(T a, T b, T epsilon = 0.005) {
    return fabs(a - b) <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

DataHandler::DataHandler() : current_slot_is_etalon(false)
{

}

void DataHandler::PrepareToRecieveData(bool etalon_)
{
    current_slot_is_etalon = etalon_;
    temp_slot.line.clear();
}

bool DataHandler::ProcessLineWithData(const QStringList &line, QString& ret)
{
    if ( line.first().compare("x=d") == 0 ) {
        return dataAquisitionHandler(line);
    } else if ( line.first().compare("x=e\n") == 0 ) {
        return dataProcessingHandler(line, ret);
    }
    ret = "broken data";
    return false;
}

bool DataHandler::dataAquisitionHandler(const QStringList &line)
{
    if (line.size() == 4)
    {
        //        qDebug() << "data " << line.at(1).toFloat() <<" "
        //                 <<line.at(3).toFloat();
        auto x = line.at(1).toDouble();
        auto y = line.at(3).toDouble();// < 6600 ? line.at(3).toFloat() :
                                        //       line.at(3).toFloat()-6600;
        temp_slot.line.append(QPointF(x, y));
        //TODO move range calculation outside data
//        if (etalon && drawLines)
//        {
//            if ( rangeVal[0].x() > x )
//                rangeVal[0].setX(x);
//            if ( rangeVal[1].x() < x )
//                rangeVal[1].setX(x);
//            if ( rangeVal[0].y() > y )
//                rangeVal[0].setY(y);
//            if ( rangeVal[1].y() < y )
//                rangeVal[1].setY(y);
//        }
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
//                auto xVal = axisValueFromMCU ?
//                            temp_slot.line.at(i).x() : micrometers[i];
            auto xVal = temp_slot.line.at(i).x();
            auto yVal = m_mode == MEASURE_MODE::RELATIVE ?
                            temp_slot.line.at(i).y() :
                            temp_slot.line.at(i).y() / calibrator_data[i];
            etalonPoints->append(QPointF( xVal, yVal ) );
        }
        qDebug() << "set etalon";
        emit activateEditBar();
    }
    else
    {
        ///calibrate and antialiasing
        QVector<QPointF> calibratedSeries;
        ///calibrate
        for (int i=0; i < temp_slot.line.size(); i++)
        {
//                auto xVal = axisValueFromMCU ?
//                            temp_slot.line.at(i).x() : micrometers[i];
            calibratedSeries.append(
                QPointF(temp_slot.line.at(i).x(),
                 temp_slot.line.at(i).y() / etalonPoints->at(i).y()*100.0));
        }
        ///antialiasing
        if ( 0 ) {
            for ( int i = 0; i < calibratedSeries.size(); i++ ) {
                //koeffs
                int k,l,m,n,o,p;
                o = (i-3 >= 0) ? i-3: 0;
                k = (i-2 >= 0) ? i-2: 0;
                l = (i-1 >= 0) ? i-1: 0;
                m = (i+1 < calibratedSeries.size()) ? (i+1) : (calibratedSeries.size()-1);
                n = (i+2 < calibratedSeries.size()) ? (i+2) : (calibratedSeries.size()-1);
                p = (i+3 < calibratedSeries.size()) ? (i+3) : (calibratedSeries.size()-1);

                calibratedSeries[i].ry() =
                            (calibratedSeries[k].y() + calibratedSeries[l].y() +
                             calibratedSeries[m].y() + calibratedSeries[i].y() +
                             calibratedSeries[n].y() + calibratedSeries[o].y() +
                             calibratedSeries[p].y())/7;
            }
        }


//            delete currentPoints;
        *currentPoints = calibratedSeries;
        //adjust borders
        qreal xMin = std::numeric_limits<qreal>::max(); // everything is <= this
        qreal xMax = std::numeric_limits<qreal>::min(); // everything is >= this
        qreal yMin = std::numeric_limits<qreal>::max();
        qreal yMax = std::numeric_limits<qreal>::min();
        foreach (QPointF p,  *currentPoints) {
            xMin = qMin(xMin, p.x());
            xMax = qMax(xMax, p.x());
            yMin = qMin(yMin, p.y());
            yMax = qMax(yMax, p.y());
        }
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
    }
//        qDebug() << rangeVal[0] << rangeVal[1];
//        m_data.append(*currentPoints);
//        lines.insert(currentSeries, m_data.back());
//        emit adjustAxis(rangeVal[0], rangeVal[1]);
//        update(currentSeries);
//        qDebug() << "end";// << *currentPoints;


    //cumulative process
    if (cumulativeMode && numberCumulativeLines != 0 )
    {
        //calc incremental average
        numberCumulativeLines++;
        float val=0;
        auto pair = vectorLines.rbegin();
        auto prevSeries = (*pair).first;
        auto prevData = (*pair).second;

        for (int i=0; i< temp_slot.line.size(); i++ )
        {
            auto An = cumulativePoints.at(i).y();
            auto xi = temp_slot.line.at(i).y();
            val = An + ( ( xi - An) / numberCumulativeLines );
            QPointF p(temp_slot.line.at(i).x(), val);
            prevData[i] = (*currentPoints)[i];
            cumulativePoints[i] = p;
            (*currentPoints)[i] = p;
        }
        m_data.pop_back();
        m_data.append(prevData);
        if (prevSeries && lines.contains(prevSeries))
            lines[prevSeries] = m_data.back();
        update(prevSeries, NULL);
    }
    else if(!current_slot_is_etalon)
    {
        numberCumulativeLines = 1;
        cumulativePoints.clear();
        for (auto& point : *currentPoints)
        {
            cumulativePoints.push_back(point);
        }
    }

    if (!current_slot_is_etalon || (current_slot_is_etalon && drawLines))
    {
        m_data.append(*currentPoints);
        vectorLines.push_back(
             QPair<QtCharts::QAbstractSeries*, QVector<QPointF> >(
                        currentSeries.first, m_data.back()));
        lines.insert(currentSeries.first, m_data.back()); //save series
        emit adjustAxis(rangeVal[0], rangeVal[1]);
        update(currentSeries.first, currentSeries.second);
    }
}
