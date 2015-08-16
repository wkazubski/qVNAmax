#include "vna_serial.h"
#include <QtAddOnSerialPort/serialport.h>
#include <iostream>

using namespace std;

QT_USE_NAMESPACE_SERIALPORT

#define MAXPOINTS 501
#define TIMEOUT1 10
#define TIMEOUT2 100

QString device;
SerialPort *serial;
bool isOpen;
QByteArray responseData;
int readptr;

VnaSerial::VnaSerial()
{
    serial = new SerialPort();
    isOpen = false;
}

VnaSerial::~VnaSerial()
{
}

int VnaSerial::initVna(int port_init, QString device_init)
{
    device = "/dev/" + device_init;
//    cout << device.toStdString() << " VNA init" <<endl;
    return(openSerialPort());
}

void VnaSerial::initMeasure(unsigned int start, unsigned int step, int points, int mode)
{
//    cout << "Start: " << start << " Step: " << step << " points:" << points << " mode: " << mode << endl;
    SendParam(mode);
    SendParam(start);
    SendParam(points);
    SendParam(step);
    if (serial->waitForBytesWritten(TIMEOUT1))
    {
        // read response
        if (serial->waitForReadyRead(TIMEOUT2))
        {
            responseData = serial->readAll();
            while (serial->waitForReadyRead(TIMEOUT1))
            {
                responseData += serial->readAll();
            }
        }
        readptr = 0;
    }
}

void VnaSerial::setFrequency(unsigned int n)
{

}

void VnaSerial::getData(int n, int *return_loss, int *angle, int *gain1, int *gain2, int *gain3)
{
//    bool result = serial->waitForReadyRead(100);
    *angle = getTwoBytes();
    *return_loss = getTwoBytes();
    if (n >= 2)
    {
        *gain1 = getTwoBytes();
        if (n >= 3)
        {
            *gain2 = getTwoBytes();
            if (n >= 4)
                *gain3 = getTwoBytes();
        }
    }
}

void VnaSerial::closePort()
{
    closeSerialPort();
}

int VnaSerial::getError()
{
    return (serial->error());
}

int VnaSerial::openSerialPort()
{
    int result = 0;
//    cout << "setting " << device.toStdString() << endl;
    serial->setPort(device);
//    cout << "set " << device.toStdString() << endl;
    if (serial->open(QIODevice::ReadWrite))
    {
//        cout << "opened" << endl;
        isOpen = true;
        if (serial->setRate(SerialPort::Rate115200)
                && serial->setDataBits(SerialPort::Data8)
                && serial->setParity(SerialPort::NoParity)
                && serial->setStopBits(SerialPort::OneStop)
                && serial->setFlowControl(SerialPort::NoFlowControl))
        {
//            cout << "OK" << endl;
        }
        else
        {
            serial->close();
            isOpen = false;
            result = -1;
        }
    }
    else
    {
        result = -2;
    }
//    cout << result << endl;
    return (result);
}

void VnaSerial::closeSerialPort()
{
    if (isOpen)
        serial->close();
    isOpen = false;
/*
    ui->statusBar->showMessage(tr("Disconnected"));
*/
}

void VnaSerial::SendParam(unsigned int x)
{
    QByteArray s = QByteArray::number(x);
//    cout << "Out: " << s.data() << endl;
    serial->write(s + "\r");
}

unsigned int VnaSerial::getTwoBytes()
{
    quint8 aa, bb;
    aa = responseData.at(readptr++);
    bb = responseData.at(readptr++);
    return(aa + 256 * bb);
}
