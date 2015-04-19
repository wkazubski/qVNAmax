#include "vna_test.h"
#include <iostream>

using namespace std;

int port_t;

VnaTest::VnaTest()
{

}

VnaTest::~VnaTest()
{

}

int VnaTest::initVna(int port_init, QString device_init)
{
    port_t = port_init;
    cout << port_t << " VNA init" <<endl;
    return (0);
}

void VnaTest::initMeasure(unsigned int start, unsigned int step, int points, int mode)
{
    cout << "Scan start" << endl;
}

void VnaTest::setFrequency(unsigned int n)
{
    cout << port_t << " VNA set to:" << n << endl;
}

void VnaTest::getData(int n, int *return_loss, int *angle, int *gain1, int *gain2, int *gain3)
{
    *amplitude = 200;
    *phase = 100;
}

void VnaTest::closePort()
{

}

int VnaTest::getError()
{
    return (0);
}

void VnaTest::setToTransmittance(bool transm)
{
    if (transm)
        cout << "Set to transmittance measurement" << endl;
    else
        cout << "Set to impedance measurement" << endl;
}
