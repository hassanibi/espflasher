#ifndef ELFFILE_H
#define ELFFILE_H

#include <QMap>
#include <QString>
#include <QObject>

namespace ESPFlasher {


class ELFFile: QObject
{
    Q_OBJECT

public:
    ELFFile(const QString &name, QObject *parent = 0);

    int getSymbolAddr(const QString &symbole);
    int getEntryPoint();

    QByteArray loadSection(const QString &section);

private:
    void fetchSymbols();

private:
    QString m_name;
    QMap<QString, int> m_symbols;
};

} //namespace ESPFlasher

#endif // ELFFILE_H
