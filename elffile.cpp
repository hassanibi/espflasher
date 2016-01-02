#include "elffile.h"

#include <QProcess>
#include <QDebug>
#include <QTemporaryFile>

namespace ESPFlasher {


ELFFile::ELFFile(const QString &name, const QString &tcPath, QObject *parent):
    QObject(parent),
    m_name(name),
    m_tcPath(tcPath)
{
}

bool ELFFile::fetchSymbols()
{
    if(!m_symbols.isEmpty())
        return true;

    QString toolNM = m_tcPath + "/xtensa-lx106-elf-nm";
    if(qgetenv("XTENSA_CORE") == "lx106")
        toolNM = "xt-nm";

    QProcess *nm = new QProcess(this);
    nm->start(toolNM, QStringList() << m_name);

    if (!nm->waitForStarted() || !nm->waitForFinished()){
        emit elfError(QString("Error calling %1, do you have Xtensa toolchain in PATH?").arg(toolNM));
        return false;
    }

    nm->setReadChannel(QProcess::StandardOutput);
    while (nm->canReadLine())
    {
        QByteArray line = nm->readLine().replace("\n", "").simplified();
        QList<QByteArray> fields = line.split(' ');
        if(fields.at(0) == "U"){
            continue;
        }
        m_symbols.insert(fields.at(2), fields.at(0).toUInt(0, 16));
    }

    return true;
}

quint32 ELFFile::getSymbolAddr(const QString &symbole)
{
    fetchSymbols();

    return m_symbols.value(symbole);
}

quint32 ELFFile::getEntryPoint()
{
    QString toolReadELF = m_tcPath + "/xtensa-lx106-elf-readelf";
    if(qgetenv("XTENSA_CORE") == "lx106")
        toolReadELF = "xt-readelf";

    QProcess *readELF = new QProcess(this);
    readELF->start(toolReadELF, QStringList() << "-h" << m_name);

    if (!readELF->waitForStarted() || !readELF->waitForFinished()){
        emit elfError(QString("Error calling %1, do you have Xtensa toolchain in PATH?").arg(toolReadELF));
        return -1;
    }

    readELF->setReadChannel(QProcess::StandardOutput);
    while (readELF->canReadLine())
    {
        QByteArray line = readELF->readLine().replace("\n", "").simplified();
        QList<QByteArray> fields = line.split(' ');
        if(fields.at(0) == "Entry"){
            return fields.at(3).toUInt(0, 16);
        }
    }

    return true;
}

QByteArray ELFFile::loadSection(const QString  &section)
{
    QString toolObjcopy = m_tcPath + "/xtensa-lx106-elf-objcopy";
    if(qgetenv("XTENSA_CORE") == "lx106")
        toolObjcopy = "xt-objcopy";

    QTemporaryFile tmpsection("XXXXXX.section");
    if (tmpsection.open())
    {
        QProcess *objcopy = new QProcess(this);
        objcopy->start(toolObjcopy, QStringList() << "--only-section" << section << "-Obinary" << m_name << tmpsection.fileName() );

        if (!objcopy->waitForStarted() || !objcopy->waitForFinished()){
            emit elfError(QString("Error calling %1, do you have Xtensa toolchain in PATH?").arg(toolObjcopy));
            return QByteArray();
        }

        QByteArray data = tmpsection.readAll();
        tmpsection.remove();
        return data;
    }

    return QByteArray();
}

} //namespace ESPFlasher

