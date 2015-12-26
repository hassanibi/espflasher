#include "elffile.h"

#include <QProcess>
#include <QDebug>
#include <QTemporaryFile>

namespace ESPFlasher {


ELFFile::ELFFile(const QString &name, QObject *parent): QObject(parent),
    m_name(name)
{
}

void ELFFile::fetchSymbols()
{
    if(!m_symbols.isEmpty())
        return;

    QString toolNM = "/opt/Espressif/crosstool-NG/builds/xtensa-lx106-elf/bin/xtensa-lx106-elf-nm";
    if(qgetenv("XTENSA_CORE") == "lx106")
        toolNM = "xt-nm";

    QProcess *nm = new QProcess(this);
    nm->start(toolNM, QStringList() << m_name);

    if (!nm->waitForStarted() || !nm->waitForFinished()){
        //Error calling %s, do you have Xtensa toolchain in PATH?
        return;
    }

    nm->setReadChannel(QProcess::StandardOutput);
    while (nm->canReadLine()) {
        QByteArray line = nm->readLine().replace("\n", "").simplified();
        QList<QByteArray> fields = line.split(' ');
        if(fields.at(0) == "U"){
            //Warning: ELF binary has undefined symbol %s
            continue;
        }
        m_symbols.insert(fields.at(2), fields.at(0).toInt(0, 16));
    }
}

int ELFFile::getSymbolAddr(const QString &symbole)
{
    fetchSymbols();

    return m_symbols.value(symbole);
}

int ELFFile::getEntryPoint()
{
    QString toolReadELF = "/opt/Espressif/crosstool-NG/builds/xtensa-lx106-elf/bin/xtensa-lx106-elf-readelf";
    if(qgetenv("XTENSA_CORE") == "lx106")
        toolReadELF = "xt-readelf";

    QProcess *readELF = new QProcess(this);
    readELF->start(toolReadELF, QStringList() << "-h" << m_name);

    if (!readELF->waitForStarted() || !readELF->waitForFinished()){
        //Error calling %s, do you have Xtensa toolchain in PATH?
        return -1;
    }

    readELF->setReadChannel(QProcess::StandardOutput);
    while (readELF->canReadLine()) {
        QByteArray line = readELF->readLine().replace("\n", "").simplified();
        QList<QByteArray> fields = line.split(' ');
        if(fields.at(0) == "Entry"){
            return fields.at(3).toInt(0, 16);
        }
    }
}

QByteArray ELFFile::loadSection(const QString  &section)
{
    QString toolObjcopy = "/opt/Espressif/crosstool-NG/builds/xtensa-lx106-elf/bin/xtensa-lx106-elf-objcopy";
    if(qgetenv("XTENSA_CORE") == "lx106")
        toolObjcopy = "xt-objcopy";

    QTemporaryFile tmpsection("XXXXXX.section");
    if (tmpsection.open()) {

        QProcess *objcopy = new QProcess(this);
        objcopy->start(toolObjcopy, QStringList() << "--only-section" << section << "-Obinary" << m_name << tmpsection.fileName() );

        if (!objcopy->waitForStarted() || !objcopy->waitForFinished()){
            //Error calling %s, do you have Xtensa toolchain in PATH?
            return QByteArray();
        }

        QByteArray data = tmpsection.readAll();
        tmpsection.remove();
        return data;
    }

}

} //namespace ESPFlasher

