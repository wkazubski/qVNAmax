#include "vna.h"
#if QT_VERSION >= 0x050000
#include <QSerialPort>
#else
#include <serialport.h>
#include <serialport-global.h>
#endif
#include <QMainWindow>

#if QT_VERSION < 0x050000
QT_BEGIN_NAMESPACE_SERIALPORT
class SerialPort;
QT_END_NAMESPACE_SERIALPORT

QT_USE_NAMESPACE_SERIALPORT
#else
QT_USE_NAMESPACE
#endif

class VnaSerial: public virtual Vna
{

public:
    VnaSerial();
    virtual ~VnaSerial();
    int initVna(int port, QString device);
    void initMeasure(unsigned int start, unsigned int step, int points, int mode);
    void setFrequency(unsigned int n);
    void getData(int n, int *amplitude, int *phase, int *gain1, int *gain2, int *gain3);
    void closePort();
    int getError();
private:
    int openSerialPort();
    void closeSerialPort();
    void SendParam(unsigned int x);
    unsigned int getTwoBytes();
};
