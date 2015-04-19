#include "vna_ad9951.h"
#include <iostream>
#include <sys/io.h>
#include <cerrno>

using namespace std;

#define SETBIT(x, b)   ((x) |= (b))
#define TESTBIT(x, b)  ((x) & (b))

#define CFR1_SIZE    32
#define CFR2_SIZE    24
#define ASF_SIZE     16
#define ARR_SIZE      8
#define FTW0_SIZE    32
#define POW0_SIZE    16
#define IBYTE_SIZE    8

#define CFR1_ADDR     0
#define CFR2_ADDR     1
#define ASF_ADDR      2
#define ARR_ADDR      3
#define FTW0_ADDR     4
#define POW0_ADDR     5

int port9;

VnaAD9951::VnaAD9951()
{

}

VnaAD9951::~VnaAD9951()
{

}

int VnaAD9951::initVna(int port_init, QString device_init)
{
    int result;
    port9 = port_init;
//    cout << port9 << "VNA init" <<endl;
    result = ioperm(0x80,3,1);
    if (result == 0)
    {
        result = ioperm(port9,3,1);
        if (result == 0)
            initDDS();
    }
    return (result);
}

void VnaAD9951::initMeasure(unsigned int start, unsigned int step, int points, int mode)
{
    setToTransmittance(mode == 1);
}

void VnaAD9951::setFrequency(unsigned int n)
{
    cout << port9 << "VNA set to:" << n << endl;
    sendCommand(FTW0_ADDR, FTW0_SIZE, n);
}

void VnaAD9951::getData(int n, int *return_loss, int *angle, int *gain1, int *gain2, int *gain3)
{
    char byte;
    int bit;

    *angle = 0;
    *return_loss = 0;

    outb(56, port9); // 111000

    for (bit=512; bit>0; bit >>=1) {
        outb_p(32, port9); // 00000 -- 32 = 100000
        byte = inb(port9+1);
//        cout << "byte:" << byte << endl;
        if (TESTBIT(byte,16))
                    SETBIT(*angle,bit);
        if (TESTBIT(byte,32))
                    SETBIT(*return_loss,bit);
        outb_p(40, port9); // 01000 clock AD -- 40 = 101000
        outb(32, port9); // 00000 -- 32 = 100000
    }
    outb(56, port9); // 111000
}

void VnaAD9951::closePort()
{

}

int VnaAD9951::getError()
{
    return (errno);
}

void VnaAD9951::setToTransmittance(bool transm)
{
    if (transm)
        outb(0x00,port9+2);
    else
        outb(0x01,port9+2);
}

void VnaAD9951::initDDS()
{
    sendCommand(CFR2_ADDR, CFR2_SIZE, 132);
    sendCommand(POW0_ADDR, POW0_SIZE, 0);

}

void VnaAD9951::sendCommand(int command, int size, unsigned int data)
{
    int i;
    unsigned int k;

    i = IBYTE_SIZE;
    k = 1;
    k <<= (IBYTE_SIZE - 1);
    do {
        (command & k) ? send1() : send0();
        command <<= 1;
    } while (--i);
    i = size;
    k = 1;
    k <<= (size -1);
    do {
        (data & k) ? send1() : send0();
        data >>= 1;
    } while (--i);
    send1();
    sendLE();
}

void VnaAD9951::send0()
{
    outb(0x18, port9);
    outb(0x1A, port9);
    outb(0x18, port9);
}

void VnaAD9951::send1()
{
    outb(0x19, port9);
    outb(0x1B, port9);
    outb(0x19, port9);
}

void VnaAD9951::sendLE()
{
    outb(0x18, port9);
    outb(0x1C, port9);
    //outb(0x18, port9);
}
