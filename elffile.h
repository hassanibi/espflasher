#ifndef ELFFILE_H
#define ELFFILE_H

#include <QMap>
#include <QString>
#include <QObject>

namespace ESPFlasher {


class ELFFile: public QObject
{
    Q_OBJECT

public:
    ELFFile(const QString &name, const QString &tcPath, QObject *parent = 0);

    quint32 getSymbolAddr(const QString &symbole, bool *ok = 0);
    quint32 getEntryPoint(bool *ok = 0);

    QByteArray loadSection(const QString &section);
    QMap<QString, quint32> symbols() const {return m_symbols; }

signals:
    void elfError(const QString &errorText);

private:
    bool fetchSymbols();

private:
    QString m_name;
    QString m_tcPath;
    QMap<QString, quint32> m_symbols;
};

} //namespace ESPFlasher

#endif // ELFFILE_H
