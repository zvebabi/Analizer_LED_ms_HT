#include "WinSerialPort.h"

WinSerialPort::WinSerialPort(QObject *parent) : QObject(parent), b_canReadLine(false),
    b_stop(false)
{}

WinSerialPort::~WinSerialPort()
{
    while(!finished)
        std::this_thread::yield();
    CloseHandle(hSerial);
}

bool WinSerialPort::open()
{
    LPCTSTR pName = (LPCTSTR)c_portName.utf16();//toStdString().c_str();
    //open port
    hSerial = CreateFile(pName,GENERIC_READ | GENERIC_WRITE,0,0,OPEN_EXISTING,
//                         FILE_ATTRIBUTE_NORMAL,0); //sync mode
                         FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, 0);   //async mode
    if(hSerial == INVALID_HANDLE_VALUE)
    {
        if(GetLastError() == ERROR_FILE_NOT_FOUND)
            qDebug() << "Port not found!";
        else
            qDebug() << "Can't connect to port!";
        b_stop =true;
        return false;
    }
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        qDebug() << "Getting state error\n";
        b_stop = true;
        return false;
    }
    //set parameters
    dcbSerialParams.BaudRate=CBR_115200;
    dcbSerialParams.ByteSize=8;
    dcbSerialParams.StopBits=ONESTOPBIT;
    dcbSerialParams.Parity=NOPARITY;
    if(!SetCommState(hSerial, &dcbSerialParams))
    {
        qDebug() << "Error setting serial port state\n";
        b_stop=true;
        return false;
    }
    memset(&overlapped_structure, 0, sizeof(overlapped_structure));
    overlapped_structure.Offset = 4096;
    overlapped_structure.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    start();
    return true;
}

bool WinSerialPort::disconnectPort()
{
     stop();
     return CloseHandle(hSerial);
}

/* get line by line while container not empty
 */
QByteArray WinSerialPort::readLine(){
    if (lineArray.size() == 1)
        b_canReadLine = false;
    QByteArray r = 0;
    if (lineArray.size() >0)
    {
        r = lineArray.first();
        lineArray.pop_front();
    }
    return r;
}

void WinSerialPort::write(char* c)
{
    //    char data[] = "m"; //test line
    DWORD dwSize = sizeof(c);

    b_Write = true;
    WriteFile(hSerial,c, dwSize, NULL, &overlapped_structure);
    while (!HasOverlappedIoCompleted(&overlapped_structure))
        std::this_thread::yield();
    b_Write = false;
}

void WinSerialPort::start()
{
    b_stop = false;
    st = new std::thread([&]() { run(); });
}

void WinSerialPort::stop()
{
    b_stop = true;
    st->join();
}

void WinSerialPort::run()
{
    finished = false;
    std::thread reader([&](){runReader();});
    reader.join();
    finished = true;
}

void WinSerialPort::runReader()
{
    //some init
    DWORD iSize;
    char sReceivedChar;

    //start main reader loop
    while(!b_stop)
    {
        if (!b_Write)
        {
            ReadFile(hSerial, &sReceivedChar, 1, &iSize,
                     &overlapped_structure);  //read byte
            while (!HasOverlappedIoCompleted(&overlapped_structure) && !b_stop)
                std::this_thread::yield();
            if (iSize > 0)  //if really readbyte -> process it
            {
                line.append(sReceivedChar);
                if (sReceivedChar == '\n')
                {
                    lineArray.push_back(line);
                    line.clear();
                    b_canReadLine = true;
                    emit readyRead();
                }
            }
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
        }
    }

}
