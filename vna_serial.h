#include "vna.h"
#include <serialport.h>
#include <serialport-global.h>
#include <QMainWindow>

QT_BEGIN_NAMESPACE_SERIALPORT
class SerialPort;
QT_END_NAMESPACE_SERIALPORT

QT_USE_NAMESPACE_SERIALPORT

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
