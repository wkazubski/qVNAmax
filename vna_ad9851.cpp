#include "vna_ad9851.h"
#include <iostream>
#include <sys/io.h>
//include <cstdint>
#include <cerrno>

#define SETBIT(x, b)   ((x) |= (b))
#define TESTBIT(x, b)  ((x) & (b))

using namespace std;

int port;

VnaAD9851::VnaAD9851()
{

}

VnaAD9851::~VnaAD9851()
{

}

int VnaAD9851::initVna(int port_init, QString device_init)
{
    int result;
    port = port_init;
//    cout << port << "VNA init" <<endl;
    result = ioperm(0x80,3,1);
    if (result == 0)
    {
        result = ioperm(port,3,1);
        if (result == 0)
            setSerialMode();
    }
    return (result);
}

void VnaAD9851::initMeasure(unsigned int start, unsigned int step, int points, int mode)
{
    setToTransmittance(mode == 1);
}

void VnaAD9851::setFrequency(unsigned int n)
{
//    cout << port << "VNA set to:" << n << endl;
    int i = 32;
    do {
        (n & 0x0001) ? send1() : send0();
        n >>= 1;
    } while (--i);
    send1();
    i=7;
    do {
        send0();
    } while (--i);
    sendLE();
}

void VnaAD9851::getData(int n, int *return_loss, int *angle, int *gain1, int *gain2, int *gain3)
{
    char byte;
    int bit;

    *angle = 0;
    *return_loss = 0;

    outb(56, port); // 111000

    for (bit=512; bit>0; bit >>=1) {
        outb_p(32, port); // 00000 -- 32 = 100000
        byte = inb(port+1);
//        cout << "byte:" << byte << endl;
        if (TESTBIT(byte,16))
                    SETBIT(*angle,bit);
        if (TESTBIT(byte,32))
                    SETBIT(*return_loss,bit);
        outb_p(40, port); // 01000 clock AD -- 40 = 101000
        outb(32, port); // 00000 -- 32 = 100000
    }
    outb(56, port); // 111000
}

void VnaAD9851::closePort()
{

}

int VnaAD9851::getError()
{
    return (errno);
}

void VnaAD9851::setToTransmittance(bool transm)
{
    if (transm)
    {
//        cout << "transm" << endl;
        outb(0x00,port+2);
    }
    else
    {
//        cout << "refl" << endl;
        outb(0x01,port+2);
    }
}

void VnaAD9851::setSerialMode()
{
    send0();
    sendLE();
    setFrequency(0);
}

void VnaAD9851::send0()
{
    outb(0x18, port);
    outb(0x1A, port);
    outb(0x18, port);
}

void VnaAD9851::send1()
{
    outb(0x19, port);
    outb(0x1B, port);
    outb(0x19, port);
}

void VnaAD9851::sendLE()
{
    outb(0x18, port);
    outb(0x1C, port);
    outb(0x18, port);
}
