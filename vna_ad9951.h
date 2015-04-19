#include "vna.h"

class VnaAD9951: public virtual Vna
{

public:
    VnaAD9951();
    virtual ~VnaAD9951();
    int initVna(int port, QString device);
    void initMeasure(unsigned int start, unsigned int step, int points, int mode);
    void setFrequency(unsigned int n);
    void getData(int n, int *amplitude, int *phase, int *gain1, int *gain2, int *gain3);
    void closePort();
    int getError();
private:
    void setToTransmittance(bool transm);
    void initDDS();
    void sendCommand(int command, int size, unsigned int data);
    void send0();
    void send1();
    void sendLE();

};
