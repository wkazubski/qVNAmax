#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QWidget>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QLocale locale;

#ifndef PREFIX
#define PREFIX "/usr/local"
#endif
    QTranslator translator;
# define _xstr(s) _str(s)
# define _str(s) #s
    translator.load("qvnamax_"+locale.name()+".qm", _xstr(APP_DATA_DIR));
    a.installTranslator(&translator);

    MainWindow w;
    w.setWindowIcon(QIcon(":icons/i16/qvnamax.png"));
    w.show();

    return a.exec();
}
