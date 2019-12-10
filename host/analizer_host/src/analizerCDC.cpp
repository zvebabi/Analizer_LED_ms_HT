#include "analizerCDC.h"
#include <QtMath>
#include <memory>

AnalizerCDC::AnalizerCDC(QObject *parent) :  QObject(parent),
    m_serNumber(-1), isPortOpen(false), serviceMode(false),
    relativeMode(false), cumulativeMode(false)
{
    qRegisterMetaType<QtCharts::QAbstractSeries*>();
    qRegisterMetaType<QtCharts::QAbstractAxis*>();

    device = new QSerialPort(this);
    connect(device, &QSerialPort::readyRead, this, &AnalizerCDC::readData);

    documentsPath = QDir::homePath()+QString("/Documents/");
    rangeVal.append(QPointF(0,0));
    rangeVal.append(QPointF(0,0));
}

AnalizerCDC::~AnalizerCDC()
{
    qDebug() << "analizer destructor";
    if (device != nullptr)
    {
        device->disconnect();
        delete device;
        qDebug() << "call disconnect port";
    }
}

void AnalizerCDC::initDevice(QString port)
{
    device->setPortName(port);
    device->setBaudRate(QSerialPort::Baud115200);
    device->setDataBits(QSerialPort::Data8);
    device->setParity(QSerialPort::NoParity);
    device->setStopBits(QSerialPort::OneStop);
    device->setFlowControl(QSerialPort::NoFlowControl);
    if(device->open(QIODevice::ReadWrite)){
        qDebug() << "Connected to: " << device->portName();
        device->write("i");
        device->setDataTerminalReady(true);
        device->setRequestToSend(false);
        emit sendDebugInfo("Connected to: " + device->portName());
    }
    else {
        qDebug() << "Can't open port" << port;
        emit sendDebugInfo("Can't open port" + port, 2000);
    }
    //read calibration file
    readEtalonParameters(QDir::currentPath()+"/calibrator", false);
}

void AnalizerCDC::getListOfPort()
{
    ports.clear();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        ports.push_back(info.portName());
        emit sendPortName(info.portName());
        qDebug() << info.portName();
    }
    std::stringstream ss;
    ss << "Found " << ports.size() << " ports";
    emit sendDebugInfo(QString(ss.str().c_str()), 100);
}

void AnalizerCDC::readData()
{
    qDebug() << "in readdata";
    while (device->canReadLine()) processLine(device->readLine());
}

/*!
 * @param seriesName Names of series, if eq. "etalon" data will be interpret as etalon data
 */
void AnalizerCDC::doMeasurements(const QString seriesName)
{
    emit sendDebugInfo("Start measurement");
    qDebug() << "doMeasurements";
    dh.PrepareToRecieveData(seriesName, seriesName == "etalon");
    qDebug() << "etalon" << (seriesName == "etalon");

    if(serviceMode)
        device->write("d");
    else
        device->write("m");
}

void AnalizerCDC::updateSeries(QtCharts::QAbstractSeries* line_, QString name_)
{
    if ( line_ ) {
        qDebug() << "update series " << name_;
        QtCharts::QXYSeries *xySeries =
                    static_cast<QtCharts::QXYSeries *>(line_);
        // Use replace instead of clear + append, it's optimized for performance
        xySeries->replace(dh.GetLineByName(name_));
    }
}

void AnalizerCDC::selectPath(QString pathForSave)
{
    documentsPath = pathForSave;
    // qDebug() << pathForSave;
    qDebug() << "Path to save: " << documentsPath;
}

void AnalizerCDC::saveDataToCSV(QString filename="data.csv")
{
    qDebug() << "save to csv...";
    qDebug() <<filename;
    qDebug() << documentsPath;

    QVector<DataSlot> dataToSave;
    auto n = dh.GetAllData(dataToSave);
    if ( n <= 0 ) {
        qDebug() << "Nothing to save!";
        emit sendDebugInfo("Nothing to save!");
        return;
    }

    QDir dataDir(documentsPath);
    if (!dataDir.exists())
        dataDir.mkpath(".");

    QFile outfile(QString(documentsPath+"/"+filename));
    if(!outfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        std::string err = outfile.errorString().toStdString();
        qDebug() << "can't open file\n";
        std::stringstream ss;
        ss << "Error while writing data: " << err;
        emit sendDebugInfo(ss.str().c_str(), 3000);
        return;
    }
    QTextStream f(&outfile);
    //header (line with um)
    f << "um" ;
    for (auto it = dataToSave.at(0).line.rbegin(); it != dataToSave.at(0).line.rend(); ++it) {
        f  << ";" << QLocale().toString(it->x());
    }
    f << "\n";
    //main
    //each line is one sample
    for ( const auto& series : dataToSave ) {
        //first column is a name
        f  << series.name;
        for ( auto it = series.line.rbegin(); it != series.line.rend(); ++it ) {
            f << ";" << QLocale().toString(it->y());
        }
        f << "\n";
    }

    outfile.close();
    qDebug() << "done";
    std::stringstream ss;
    ss << "Saved to " << documentsPath.toStdString()
       << "/" << filename.toStdString();
    emit sendDebugInfo(QString(ss.str().c_str()));
}

void AnalizerCDC::deleteSeries(const QString name)
{
    std::stringstream ss;
    ss << "Measurement " << name.toStdString();

    auto ok = dh.DeleteLine(name);
    if (ok) {
        ss << " deleted!";
    } else {
        ss << " does not exist!";
    }
    emit sendDebugInfo(QString(ss.str().c_str()));
//    update();
}

/*!
 * @brief Acquire data from storage. wrap and send to gui
 */
void AnalizerCDC::update()
{
    qDebug() << "in update";
    auto [legend, min_range, max_range] = dh.GetSeriesNamesToShow();

    emit updateDrawer(legend.size(), legend, min_range, max_range);
//    emit sendDebugInfo("Update done");
}

void AnalizerCDC::processLine(const QByteArray &_line)
{
    qDebug() << _line;
    QStringList line;
    for (auto w : _line.split(','))
    {
        line.append(QString(w));
    }
    bool oK = {true};
    QString status = "";
    //identity
    if ( line.first().compare("x=i") ==0 )
        identityHandler(line);
    //service mode parser
    if ( line.first().compare("x=s") == 0 )
        serviceModeHandler(line);   //parse all comands here
    //measure mode
    if ( line.first().compare("x=m\n") == 0 )
        buttonPressHandler(/*line*/);
    if ( line.first().compare("x=d") == 0 || line.first().compare("x=e\n") == 0 )
        oK = dh.ProcessLineWithData(line, status);
    if (!oK) {
        emit sendDebugInfo(status, 3000);
    } else {
        if ( status == "activateEditBar" ) {
            emit activateEditBar();
        } else if ( status == "updateDrawer" ) {
            update();
        }
    }
}

void AnalizerCDC::serviceModeHandler(const QStringList &line)
{
    if(line.at(1).compare("START") == 0)    //create file
    {
        const std::time_t t = time(nullptr);
        struct std::tm now;
        localtime_s(&now, &t);
        char buf[200];
        std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S_raw.csv", &now);
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

void AnalizerCDC::identityHandler(const QStringList &line)
{
    if (line.at(1).compare("SERIAL") == 0 )
    {
        qDebug() << "Serial# " << line.at(2).toInt();
        m_serNumber = line.at(2).toInt();
        emit sendSerialNumber(QString("%1").arg(m_serNumber, 4, 10,QChar('0')));
    }
    if(line.at(1).compare("TYPE") ==0 )
    {
        qDebug() << "Type: " << line.at(2).toInt();
        /// 0x01 - absorbance, 0x81 - transmittance
        if (line.at(2).toInt() == 0x01 )
        {
            emit sendAxisName("Reflection, %");
            emit sendDebugInfo("Connection of LMS-R minispectrometer completed!");
            emit disableButton();
        }
        else if (line.at(2).toInt() == 0x81 )
        {
            emit sendAxisName("Transmission, %");
            emit sendDebugInfo("Connection of LMS-T minispectrometer completed!");
            emit disableButton();
        }
        else
            emit sendDebugInfo("Undefined device type. Check connection",
                               10000);
    }
    emit sendDebugInfo(QString("Click calibration button to perform device calibration"));
}

void AnalizerCDC::buttonPressHandler(/*const QStringList &line*/)
{
    emit makeSeries();
    qDebug() << "signal from button";
}

void AnalizerCDC::readEtalonParameters(const QString filename, bool saveNew=true)
{
    //read calibration file
    QString readedValue;
    QFile calibfile(filename);
    if ( !calibfile.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
        std::string err = calibfile.errorString().toStdString();
        emit activateRelativeMod();
        std::stringstream ss;
        ss << "Can not read file \'"
           << QDir::currentPath().toStdString()
           << "/calibrator\'"
           << " Run application in relative mode. "
           << "Error #"
           << err;
        emit sendDebugInfo(ss.str().c_str());
        return;
    }

    QTextStream f(&calibfile);

    QFile calibfileNew;
    std::shared_ptr<QTextStream> f_out;
    if ( saveNew ) {
        calibfileNew.setFileName(QDir::currentPath()+"/calibrator");
        if(!calibfileNew.open(QIODevice::WriteOnly | QIODevice::Text))
            qDebug()<< "Cannot save new calibration parameters";
        f_out = std::make_shared<QTextStream>(&calibfileNew);
    }

    //read etalon name
    auto etalonName = f.readLine();
    emit sendEtalonName(etalonName);
    if ( saveNew )
       *f_out << etalonName << "\n";
    QVector<double> calibratorData;
    double k;
    while ( !f.atEnd() ) {
        f >> k;
        if( k > 0.00 && k < 1.00 ) {
            if ( saveNew )
                *f_out << k << " ";
            calibratorData.push_back(k);
        }
    }
    qDebug() << calibratorData;
    qDebug() << "calibratorData size: " << calibratorData.size();
    if ( calibratorData.size() >= 5 ) {
        dh.updateCalibrationParameters(calibratorData);
        emit deActivateRelativeMod();
    } else {
        emit activateRelativeMod();
        emit sendDebugInfo("Wrong file with etalon data", 2000);
    }
}



















