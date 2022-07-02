#ifndef __MINIC_H__
#define __MINIC_H__

#include <QString>
#include <QProcess>
#include <QDebug>
#include <QFile>
class Minic
{
public:
    Minic() = default;
    int Compile(QString& filename);
    int Run(QString curPos, QString& filename);
};

#endif // __MINIC_H__
