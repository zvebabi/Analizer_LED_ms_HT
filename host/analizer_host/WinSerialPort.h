#ifndef WINSERIALPORT_H
#define WINSERIALPORT_H

#include <QtCore/QObject>
#include <QDebug>
#ifdef _WIN32
#include <windows.h>
#endif
#include <QByteArray>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>


class WinSerialPort : public QObject
{
    Q_OBJECT
public:
    explicit WinSerialPort(QObject *parent = 0);
    ~WinSerialPort();
public slots:
    void setPortName(QString port) {c_portName = port;} //setter
    QString portName() { return c_portName; } //getter
    bool open();
    bool disconnectPort();
    bool canReadLine() { return b_canReadLine; } //wait flag

    QByteArray readLine();
    void write(char *c);
    void start(); //start thread
    void stop();
    void run(); //thread with reader
    void runReader(); //main reader function

signals:
    void readyRead();
private:
    HANDLE hSerial;
    OVERLAPPED overlapped_structure;
    std::thread *st;
    std::atomic_bool b_canReadLine, b_stop, finished, b_Write;
    QString c_portName;
    QVector<QByteArray> lineArray;
    QByteArray line;
};

#endif // WINSERIALPORT_H
