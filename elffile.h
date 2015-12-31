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

    quint32 getSymbolAddr(const QString &symbole);
    quint32 getEntryPoint();

    QByteArray loadSection(const QString &section);
    QMap<QString, quint32> symbols() const {return m_symbols; }

private:
    bool fetchSymbols();

private:
    QString m_name;
    QMap<QString, quint32> m_symbols;
};

} //namespace ESPFlasher

#endif // ELFFILE_H
