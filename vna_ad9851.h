#include "vna.h"

class VnaAD9851: public virtual Vna
{

public:
    VnaAD9851();
    virtual ~VnaAD9851();
    int initVna(int port, QString device);
    void initMeasure(unsigned int start, unsigned int step, int points, int mode);
    void setFrequency(unsigned int n);
    void getData(int n, int *amplitude, int *phase, int *gain1, int *gain2, int *gain3);
    void closePort();
    int getError();
private:
    void setToTransmittance(bool transm);
    void setSerialMode();
    void send0();
    void send1();
    void sendLE();

};
