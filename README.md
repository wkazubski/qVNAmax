# qVNAmax
Control program for VNAmax vector network analyzers

  qVNAmax is a software to control antenna analysers based on IW2HEV design.
This includes:
 - original analyser by IW2HEV (Funkamateur 12/2004) - parallel port,
 - its development VNA 3p2 by SP3SWJ (http://www.sp2swj.sp-qrp.pl/VNA3p2/VNA3p2.htm),
 - the above with serial/USB adapter (http://www.sp2swj.sp-qrp.pl/VNA_IF/VNA_IF.htm),
 - niniVNA (USB) - untested,
 - MAX 2, MAX 3 and MAX6 analysers by SP3SWJ (http://www.max6.pl/news.php)
 
Those analyzes can measure impedance of antenna (or other circut) connected to
its terminals in range from near 0 to 70, 180, or 500MHz, depending on DDS chip
used. Some can also measure transmitance of two port circuits (filters,
transmission lines).

qVNAmax is written in C++, using Qt4 framework (http://qt-project.org), with
Qwt wtdgets (http://qwt.sourceforge.net) for plot display. The QtSerialPort 
library (http://qt-project.org/wiki/QtSerialPort) is used for serial and USB
communication support. The software was written for GNU Linux operating system,
but can be ported to other operating systems. The software is released on 
GNU GPL version 3 license.

The supported features are:
 - measurement of reflectance of a RF circuit (SWR, return loss, impedance),
 - return loss extension to 40dB mod,
 - measurement of transmitance (if supported by analyzer),
 - RF frequency generation,
 - graphical presentation of measurement results,
 - readout at two markers,
 - indication of the minimum SWR point,
 - printing results to PDF file,
 - saving and retrieving files in several formats
    - binary (standard).
    - binary (gVNA),
    - comma separated,
 - support for parallel port, serial and USB devices,
 - support for up to 4 different devices.

Parallel port mode requires direct access to IO ports, so most likely the                                                                              
software has to be run with root privilleges. For serial / USB mode, the                                                                                 
software can (and should) be run by an unpriviledged user, providing he has                                                                              
read/write rights to given interface.                                                                                                                    
                                                                                                                                                         
For installation instruction see INSTALL file.                                                                                                         
