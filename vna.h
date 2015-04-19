#ifndef VNA_H
#define VNA_H

#include <QString>

class Vna {
public:
    virtual ~Vna(){};
    virtual int initVna(int port, QString device) = 0;
    virtual void initMeasure(unsigned int start, unsigned int step, int points, int mode) = 0;
    virtual void setFrequency(unsigned int n) = 0;
    virtual void getData(int n, int *amplitude, int *phase, int *gain1, int *gain2, int *gain3) = 0;
    virtual void closePort() = 0;
    virtual int getError() = 0;
};

#endif // VNA_H
