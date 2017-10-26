#include "analizerCDC.h"
#include <QtMath>

analizerCDC::analizerCDC(QObject *parent) : QObject(parent),
    firstLine(true), aaManual(true), serviceMode(false), isPortOpen(false),
    cumulativeMode(false), relativeMode(false), numberCumulativeLines(0)
{
    qRegisterMetaType<QtCharts::QAbstractSeries*>();
    qRegisterMetaType<QtCharts::QAbstractAxis*>();
    documentsPath = QDir::homePath()+QString("/Documents/");
    rangeVal.append(QPointF(0,0));
    rangeVal.append(QPointF(0,0));
//    isPortOpen=false;
#ifdef _WIN32
    device = new WinSerialPort(this);
    connect(device, &WinSerialPort::readyRead, this, &analizerCDC::readData);
#else
    device = new QSerialPort(this);
    connect(device, &QSerialPort::readyRead, this, &analizerCDC::readData);
#endif

    etalonPoints = new QVector<QPointF>(43,QPointF(1,1));
    drawLines = false;
}

analizerCDC::~analizerCDC()
{
    if (device != NULL)
    {
#ifdef _WIN32
        if (isPortOpen)
        {
            device->disconnectPort();
            delete device;
        }
#else
        device->disconnect();
        delete device;
#endif

    }
}

void analizerCDC::cppSlot(const QString &msg)
{
    //Найдем строки ввода
    QObject* field1 = this->parent()->findChild<QObject*>("field1");
    QObject* field2 = this->parent()->findChild<QObject*>("field2");
    //Найдем поле вывода
    QObject* textArea = this->parent()->findChild<QObject*>("textArea");

    //Считаем информацию со строк ввода через свойство text
    QString str1=(field1->property("text")).toString();
    QString str2=(field2->property("text")).toString();

    int a = str1.toInt();//Переведем строку в число
    int b = str2.toInt();//Переведем строку в число

    int c = a + b; //Вычисления наши

    QString strResult=QString::number(c);//Переведем результат в строку

    //Ну и наконец выведем в поле вывода нашу информацию
    textArea->setProperty("text", str1+" + "+str2+" = "+strResult+" "+msg);
}

void analizerCDC::initDevice(QString port)
{
#ifdef _WIN32 //windows compatibility
    device->setPortName(port);
    isPortOpen = device->open();
    if(isPortOpen){
        qDebug() << "Connected to: " << device->portName();
        emit sendDebugInfo("Connected to: " + device->portName());
    }
    else {
        qDebug() << "Can't open port" << port;
        emit sendDebugInfo("Can't open port" + port);
    }    
#else
    device->setPortName(port);
    device->setBaudRate(QSerialPort::Baud115200);
    device->setDataBits(QSerialPort::Data8);
    device->setParity(QSerialPort::NoParity);
    device->setStopBits(QSerialPort::OneStop);
    device->setFlowControl(QSerialPort::NoFlowControl);
    if(device->open(QIODevice::ReadWrite)){
        qDebug() << "Connected to: " << device->portName();
        device->write("i");
//        emit sendDebugInfo("Connected to: " + device->portName());
    }
    else {
        qDebug() << "Can't open port" << port;
        emit sendDebugInfo("Can't open port" + port, 2000);
    }    
#endif
    //read calibration file
    float k;
    std::ifstream f("calibrator");
    if(!f.is_open())
        emit sendDebugInfo("Can't read calibration parameters", 10000);
    while(f >> k)
    {
        calibratorData.push_back(k);
    }
}

void analizerCDC::getListOfPort()
{
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        ports.push_back(info.portName());
        emit sendPortName(info.portName());
        qDebug() << info.portName();
    }
//    emit sendPortName(QString("teststr"));
//    QObject* comboBox = this->parent()->findChild<QObject*>("availablePorts");
    //    comboBox->setProperty("append", info.portName());
}

void analizerCDC::readData()
{
    while (device->canReadLine()) processLine(device->readLine());
}

void analizerCDC::doMeasurements(QtCharts::QAbstractSeries *series,
                                 bool _etalon)
{
    qDebug() << "doMeasurements";
    currentPoints = new QVector<QPointF> ;
    etalon = _etalon;
    qDebug() << etalon;
#if 1
    if(serviceMode)
        device->write("d");
    else
        device->write("m");
    currentSeries = series;
#else
    auto colCount =10;
    // Append the new data depending on the type
    currentPoints->reserve(colCount);
//    srand(0);
    for (int j(0); j < colCount; j++) {
        qreal x(0);
        qreal y(0), _y(0);
      // data with sin + random component
        y = qSin(3.14159265358979 / 30 * j) + 0.5 + (qreal) rand() / (qreal) RAND_MAX;
//        y = _y < 1.5 ? _y : 0;
        x = j;//micrometers[j];
        //find borders
        if (etalon && drawLines) {
            if ( rangeVal[0].x() > x )
                rangeVal[0].setX(x);
            if ( rangeVal[1].x() < x )
                rangeVal[1].setX(x);
            if ( rangeVal[0].y() > y )
                rangeVal[0].setY(y);
            if ( rangeVal[1].y() < y )
                rangeVal[1].setY(y);
            }
        //add data to graph
        currentPoints->append(QPointF(x, y));
    }

    if (etalon)
    {
        etalonPoints = new QVector<QPointF>(*currentPoints);
    }
    else
    {
        QVector<QPointF> calibratedSeries;
        ///calibrating
        for (int i=0; i < currentPoints->size(); i++)
        {
            calibratedSeries.append(
                        QPointF(currentPoints->at(i).x(),
                      currentPoints->at(i).y() / etalonPoints->at(i).y()*100.0));
        }
        ///antialiasing
        if(aaManual)
            for(int i=0; i < calibratedSeries.size(); i++)
            {
                //koeffs
                int k,l,m,n,o,p;
                o = (i-3 >= 0) ? i-3: 0;
                k = (i-2 >= 0) ? i-2: 0;
                l = (i-1 >= 0) ? i-1: 0;
                m = (i+1 < calibratedSeries.size()) ? (i+1) : (calibratedSeries.size()-1);
                n = (i+2 < calibratedSeries.size()) ? (i+2) : (calibratedSeries.size()-1);
                p = (i+3 < calibratedSeries.size()) ? (i+3) : (calibratedSeries.size()-1);

                calibratedSeries[i] = (calibratedSeries[k] + calibratedSeries[l] +
                                      calibratedSeries[m] + calibratedSeries[i] +
                                      calibratedSeries[n] + calibratedSeries[o] +
                                       calibratedSeries[p])/7;
            }


//        delete currentPoints;
//        qDebug() << *currentPoints;
        *currentPoints = calibratedSeries;
//        qDebug() << *currentPoints;
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
    if (!etalon || (etalon && drawLines))
    {
        m_data.append(*currentPoints);
        lines.insert(series, m_data.back()); //save series
        emit adjustAxis(rangeVal[0], rangeVal[1]);
        update(series);
    }

#endif

}

void analizerCDC::saveDataToCSV(QString filename="data.csv")
{
    qDebug() << "save to csv..";
    qDebug() <<filename;
    qDebug() << documentsPath;

    QDir dataDir(documentsPath);
    if (!dataDir.exists())
        dataDir.mkpath(".");

    std::fstream f(QString(documentsPath+"/"+filename).toStdString(),
                   std::fstream::out);
    if (!f.is_open())
        qDebug() << "can't open file\n";
    int i = 1;
    for (auto series : lines.keys())
    {
        f << "Sample: " << series->name().toStdString() << "\n";
        for ( auto& p : lines.value(series))
            f << p.x() << ", " << p.y() << "\n";
        ++i;
    }
    f.close();
    qDebug() << ".done";
}

void analizerCDC::deleteSeries(QtCharts::QAbstractSeries *series)
{
    if (lines.contains(series))
        lines.remove(series);
    //recalc axis
    std::vector<float> xVals;
    std::vector<float> yVals;
    if (lines.size() <= 0)
        return;

    for (auto series : lines.keys())
    {
        for ( auto& points : lines.value(series))
        {
            xVals.push_back(points.x());
            yVals.push_back(points.y());
        }
    }
    std::sort(xVals.begin(),xVals.end());
    std::sort(yVals.begin(),yVals.end());
//    qDebug() << "start " << *(xVals.begin()) << " stop " << *(xVals.rbegin());
//    qDebug() << "start " << *(yVals.begin()) << " stop " << *(yVals.rbegin());
    rangeVal[0].setX( *(xVals.begin() ) );
    rangeVal[1].setX( *(xVals.rbegin()) );
    rangeVal[0].setY( *(yVals.begin() ) );
    rangeVal[1].setY( *(yVals.rbegin()) );
    emit adjustAxis(rangeVal[0], rangeVal[1]);
}

void analizerCDC::update(QtCharts::QAbstractSeries *series)
{
    if (series && lines.contains(series)) {
        QtCharts::QXYSeries *xySeries =
                static_cast<QtCharts::QXYSeries *>(series);
//        QVector<QPointF> points = lines.value(series);
        // Use replace instead of clear + append, it's optimized for performance
        xySeries->replace(lines.value(series));

    //fill barSeries
        QVariantList barData;
        QStringList barAxis;
        foreach (auto p, lines.value(series)) {
            barData.append(p.y());
            barAxis.append(QString::number(p.x()));
        }
        updateBarSeries(xySeries->name(),barData, xySeries->color(), barAxis);
    }
}

void analizerCDC::processLine(const QByteArray &_line)
{
//    QByteArray line = device->readAll();
//    qDebug() << _line;
    QStringList line;//(_line);
    for (auto w : _line.split(','))
    {
        line.append(QString(w));
    }
//identity
    if( line.first().compare("x=i") ==0)
        identityHandler(line);
//service mode parser
    if( line.first().compare("x=s") == 0)
        serviceModeHandler(line);   //parse all comands here
//measure mode
    if( line.first().compare("x=m\n") == 0)
        buttonPressHandler(line);
    if( line.first().compare("x=d") ==0)
        dataAquisitionHandler(line);
    if( line.first().compare("x=e\n") ==0)
        dataProcessingHandler(line);
}

void analizerCDC::serviceModeHandler(const QStringList &line)
{
//    qDebug() << line.at(1);
//    qDebug() << line.at(1).compare("START");
    if(line.at(1).compare("START") == 0)    //create file
    {
        std::time_t  t = time(0);
        struct std::tm * now = localtime( & t );
        char buf[200];
        std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S_raw.csv", now);
        QString filename(buf);
        diagnosticLog.open(QString(documentsPath+"/"+filename).toStdString(),
                       std::fstream::out);
        if (!diagnosticLog.is_open())
            qDebug()<< "Can't open file!";
        qDebug() << "start file";
    }
    if(line.at(1).compare("END") == 0)      //close file
    {
        if (diagnosticLog.is_open())
            diagnosticLog.close();
        qDebug() << "endfile";
    }
    if(line.at(1).compare("LED") == 0)      //insert line with led wavelenght
    {
        diagnosticLog << "\nLed#" << line.at(2).toStdString()
                      << " (" << line.at(3).toFloat() << " um)\n"
                      << "Signal,Background\n";
        qDebug() << "Led#"<<line.at(2) << " (" <<line.at(3) << "um)";
    }
    if(line.at(1).compare("DATA") == 0)     //insert line with data
    {
        diagnosticLog << std::fixed << line.at(2).toFloat() << ", "
                      << std::fixed << line.at(3).toFloat() << "\n";
        qDebug() << "signal: "<<line.at(2) << ", bgd: " <<line.at(3);
    }
}

void analizerCDC::identityHandler(const QStringList &line)
{
    if (line.at(1).compare("SERIAL") == 0 )
        qDebug() << "Serial# " << line.at(2).toInt();
    if(line.at(1).compare("TYPE") ==0 )
    {
        qDebug() << "Type: " << line.at(2).toInt();
        /// 0x01 - absorbance, 0x81 - transmittance
        if (line.at(2).toInt() == 0x01 )
        {
            emit sendAxisName("Absorbance");
            emit sendDebugInfo("Connected to absorbance minispectrometer");
        }
        else if (line.at(2).toInt() == 0x81 )
        {
            emit sendAxisName("Transmittance");
            emit sendDebugInfo("Connected to transmittance minispectrometer");
        }
        else
            emit sendDebugInfo("Undefined device type. Check connection",
                               10000);
    }
}

void analizerCDC::dataAquisitionHandler(const QStringList &line)
{
    if (line.size() == 4)
    {
        //        qDebug() << "data " << line.at(1).toFloat() <<" "
        //                 <<line.at(3).toFloat();
        auto x = line.at(1).toFloat();
        auto y = line.at(3).toFloat() < 6600 ? line.at(3).toFloat() :
                                               line.at(3).toFloat()-6600;
        currentPoints->append(QPointF(x, y));
        if (etalon && drawLines)
        {
            if ( rangeVal[0].x() > x )
                rangeVal[0].setX(x);
            if ( rangeVal[1].x() < x )
                rangeVal[1].setX(x);
            if ( rangeVal[0].y() > y )
                rangeVal[0].setY(y);
            if ( rangeVal[1].y() < y )
                rangeVal[1].setY(y);
        }
    }
}

void analizerCDC::dataProcessingHandler(const QStringList &line)
{

    if (currentPoints->rbegin()->y() == -1.0)
        emit sendDebugInfo("Bad data", 5000);
    //save series and data for future
    if (etalon)
    {
        etalonPoints = new QVector<QPointF>;//(*currentPoints);
        for (int i=0; i < currentPoints->size(); i++)
        {
//                auto xVal = axisValueFromMCU ?
//                            currentPoints->at(i).x() : micrometers[i];
            auto xVal = currentPoints->at(i).x();
            auto yVal = relativeMode ?
                            currentPoints->at(i).y() :
                            currentPoints->at(i).y() / calibratorData[i];
            etalonPoints->append(QPointF( xVal, yVal ) );
        }
        qDebug() << "set etalon";
    }
    else
    {
        ///calibrate and antialiasing
        QVector<QPointF> calibratedSeries;
        ///calibrate
        for (int i=0; i < currentPoints->size(); i++)
        {
//                auto xVal = axisValueFromMCU ?
//                            currentPoints->at(i).x() : micrometers[i];
            calibratedSeries.append(
                QPointF(currentPoints->at(i).x(),
                 currentPoints->at(i).y() / etalonPoints->at(i).y()*100.0));
        }
        ///antialiasing
        if(0)
            for(int i=0; i < calibratedSeries.size(); i++)
            {
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

        for (int i=0; i< currentPoints->size(); i++ )
        {
            auto An = cumulativePoints.at(i).y();
            auto xi = currentPoints->at(i).y();
            val = An + ( ( xi - An) / numberCumulativeLines );
            QPointF p(currentPoints->at(i).x(), val);
            prevData[i] = (*currentPoints)[i];
            cumulativePoints[i] = p;
            (*currentPoints)[i] = p;
        }
        m_data.pop_back();
        m_data.append(prevData);
        if (prevSeries && lines.contains(prevSeries))
            lines[prevSeries] = m_data.back();
        update(prevSeries);
    }
    else if(!etalon)
    {
        numberCumulativeLines = 1;
        cumulativePoints.clear();
        for (auto& point : *currentPoints)
        {
            cumulativePoints.push_back(point);
        }
    }

    if (!etalon || (etalon && drawLines))
    {
        m_data.append(*currentPoints);
        vectorLines.push_back(
             QPair<QtCharts::QAbstractSeries*, QVector<QPointF> >(
                        currentSeries, m_data.back()));
        lines.insert(currentSeries, m_data.back()); //save series
        emit adjustAxis(rangeVal[0], rangeVal[1]);
        update(currentSeries);
    }
}

void analizerCDC::buttonPressHandler(const QStringList &line)
{
    emit makeSeries();
    qDebug() << "signal from button";
}


















