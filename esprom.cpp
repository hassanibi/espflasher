#include "esprom.h"
#include "tools.h"

#include <QThread>
#include <QDataStream>
#include <QDebug>

namespace ESPFlasher {

ESPRom::ESPRom(QObject *parent):
    QSerialPort(parent),
    m_waitTimeout(500),
    m_isSync(false),
    m_flashID(0),
    m_resetMode(1)
{

}

ESPRom::ESPRom(const QString &portName, const QSerialPort::BaudRate baudRate, int resetMode, QObject *parent) :
    QSerialPort(parent),
    m_waitTimeout(500),
    m_isSync(false),
    m_flashID(0),
    m_resetMode(resetMode)
{
    setPortName(portName);
    setBaudRate(baudRate);

    connect(this, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(this, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleSerialError(QSerialPort::SerialPortError)));
}

ESPRom::~ESPRom()
{
    if (isOpen())
        close();
}

bool ESPRom::openPort()
{
    if(isPortOpen())
        return true;

    setDataBits(QSerialPort::Data8);
    setParity(QSerialPort::NoParity);
    setStopBits(QSerialPort::OneStop);
    setFlowControl(QSerialPort::NoFlowControl);


    if (open(QIODevice::ReadWrite)) {

        for(int i = 0; i < 4; i++){

            resetDevice(m_resetMode);

            for(int i = 0; i < 4; i++){
                if(sync()){
                    return true;
                }
            }
        }
    }

    if(!m_isSync){
        closePort();
    }

    return false;
}

void ESPRom::resetDevice(int mode)
{
    switch (static_cast<ResetMode>(mode))
    {
    case Auto:
        setDataTerminalReady(false);
        setRequestToSend(true);
        QThread::msleep(1);

        setDataTerminalReady(true);
        QThread::msleep(1);

        setDataTerminalReady(false);
        QThread::msleep(100);

        setRequestToSend(false);

        break;

    case CK:
        setDataTerminalReady(true);
        setRequestToSend(true);
        QThread::msleep(5);

        setRequestToSend(false);
        QThread::msleep(75);

        setDataTerminalReady(false);

        break;

    case Wifio:
        setDataTerminalReady(false);
        setDataTerminalReady(true);
        QThread::msleep(5);

        setDataTerminalReady(false);

        setBreakEnabled(true);
        QThread::msleep(250);
        setBreakEnabled(false);
        QThread::msleep(250);

        break;

    case NodeMCU:
        setDataTerminalReady(false);
        setRequestToSend(true);
        QThread::msleep(5);

        setDataTerminalReady(true);
        setRequestToSend(false);
        QThread::msleep(75);

        setRequestToSend(true);

        break;

    case DTROnly:
        setDataTerminalReady(true);
        QThread::msleep(100);
        setDataTerminalReady(false);
        break;

    default:
        break;
    }
}

bool ESPRom::sync()
{

    QByteArray packet("\x07\x07\x12\x20");
    for(int i = 0; i < 32; i++){
        packet.append("\x55", 1);
    }

    if(sendCommand(Sync, packet).isValid()){
        for(int i = 0; i < 8; i++){
            sendCommand();
        }
        m_isSync = true;
        return true;
    }

    return false;
}

void ESPRom::closePort()
{
    if(isOpen()){
        m_isSync = false;
        close();
    }
}

qint64	ESPRom::readData(char * data, qint64 maxSize)
{
    return QSerialPort::readData(data, maxSize);
}

qint64	ESPRom::writeData(const char * data, qint64 maxSize)
{
    return QSerialPort::writeData(data, maxSize);
}

void ESPRom::handleSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        close();

    }

    if(error != QSerialPort::NoError){
        emit commandError(errorString());
    }
}

QByteArray ESPRom::readBytes(int size)
{
    waitForReadyRead(10);

    QByteArray bytes = read(size);

    return bytes;
}

QByteArray ESPRom::readAndEscape(int size)
{    
    qint64 readCount = 0;
    QByteArray data;
    while((data.size() < size))
    {
        waitForReadyRead(10);

        char buffer[1];
        readCount += read(buffer, 1);
        if (buffer[0] == '\xdb')
        {
            readCount += read(buffer, 1);
            if (buffer[0] == '\xdc')
                data.append('\xc0');
            else if (buffer[0] == '\xdd')
                data.append('\xdb');
            else{
                qDebug() << "Invalid SLIP escape";
                return data;
            }
        }
        else{
            data.append(buffer, 1);
        }
    }

    return data;
}

void ESPRom::writeToPort(QByteArray data)
{
    QByteArray buf = "\xc0" + (data.replace("\xdb","\xdb\xdd").replace("\xc0","\xdb\xdc")) + "\xc0";
    write(buf.data(), buf.size());
    if (!waitForBytesWritten(m_waitTimeout)) {
        //qDebug() << "Wait write response timeout";
        return;
    }
}

QString ESPRom::errorText(CommandResponse response)
{
    switch (response.error()) {
    case CommandResponse::InvalidResponse:
        return QString::asprintf("Invalid response 0x%02x to command", response.value);
        break;
    case CommandResponse::InvalidPacketHead:
        return QLatin1String("Invalid head of packet");
        break;
    case CommandResponse::InvalidPacketEnd:
        return QLatin1String("Invalid end of packet");
        break;
    default:
        break;
    }

    return QString();
}

CommandResponse ESPRom::sendCommand(ESPCommand cmd, const char *data, quint16 size, quint32 chk)
{
    emit commandStarted(cmd);

    if(cmd != NoCommand){
        QByteArray buffer;
        buffer.append('\0');
        buffer.append((char)cmd);

        char sizeBytes[2];
        quint16toBytes(size, sizeBytes);
        buffer.append(sizeBytes, 2);

        char chkBytes[4];
        quint32toBytes(chk, chkBytes);
        buffer.append(chkBytes, 4);
        buffer.append(data, size);

        writeToPort(buffer);

        if(waitForReadyRead(m_waitTimeout)){
            //TODO:
        }
    }

    int retries = 100;
    CommandResponse response;
    while (retries > 0){
        response = receiveResponse();

        if(cmd == NoCommand || response.cmd == (quint8)cmd){
            emit commandFinished(cmd);
            return response;
        }
        retries--;
    }

    if(cmd != NoCommand && cmd != Sync){
        emit commandError(errorText(response));
    }

    return CommandResponse::InvalidResponse;
}

CommandResponse ESPRom::sendCommand(ESPCommand cmd, const QByteArray &data, quint32 chk)
{
    return sendCommand(cmd, data.data(), data.size(), chk);
}

CommandResponse ESPRom::receiveResponse()
{
    if(readBytes(1) != QByteArray("\xc0", 1)){
        return CommandResponse::InvalidPacketHead;
    }

    QByteArray bytes = readAndEscape(8);
    if(bytes.isEmpty() || bytes.at(0) != 0x01){
        return CommandResponse::InvalidResponse;
    }

    CommandResponse response;
    response.cmd = bytes.at(1);
    response.size = bytes2quint16(&bytes.data()[2]);
    response.value = bytes2quint32(&bytes.data()[4]);
    response.body = readAndEscape(response.size);

    if(readBytes(1) != QByteArray("\xc0", 1)){
        return CommandResponse::InvalidPacketEnd;
    }

    return response;
}

quint32 ESPRom::readReg(quint32 addr)
{
    char bytes[4];
    quint32toBytes(addr, bytes);

    CommandResponse response = sendCommand(ReadReg, bytes, 4);
    if(!response.isValid()){
        emit commandError("Failed to read target memory");
        return 0x0;
    }

    return response.value;
}

bool ESPRom::writeReg(quint32 addr, quint32 value, quint32 mask, quint32 delayus)
{
    char bytes[16];
    quint32toBytes(addr, &bytes[0]);
    quint32toBytes(value, &bytes[4]);
    quint32toBytes(mask, &bytes[8]);
    quint32toBytes(delayus, &bytes[12]);

    if(!sendCommand(WriteReg, bytes, 16).isValid()){
        emit commandError("Failed to write target memory");
        return false;
    }

    return true;
}

bool ESPRom::memBegin(quint32 size, quint32 blocks, quint32 blocksize, quint32 offset)
{
    char bytes[16];
    quint32toBytes(size, &bytes[0]);
    quint32toBytes(blocks, &bytes[4]);
    quint32toBytes(blocksize, &bytes[8]);
    quint32toBytes(offset, &bytes[12]);

    if(!sendCommand(MemBegin, bytes, 16).isValid()){
        emit commandError("Failed to enter RAM download mode");
        return false;
    }

    return true;
}

bool ESPRom::memBlock(const QByteArray &data, quint32 seq)
{
    char bytes[16];
    quint32toBytes(data.size(), &bytes[0]);
    quint32toBytes(seq, &bytes[4]);
    quint32toBytes(0, &bytes[8]);
    quint32toBytes(0, &bytes[12]);

    QByteArray packet;
    packet.append(bytes, 16);
    packet.append(data);

    if(!sendCommand(MemData, packet, Tools::checksum(data)).isValid()){
        emit commandError("Failed to write to target RAM");
        return false;
    }

    return true;
}

bool ESPRom::memFinish(quint32 entrypoint)
{
    char bytes[8];
    quint32toBytes((quint32)(entrypoint == 0), &bytes[0]);
    quint32toBytes(entrypoint, &bytes[4]);

    if(!sendCommand(MemEnd, bytes, 8).isValid()){
        emit commandError("Failed to leave RAM download mode");
        return false;
    }

    return true;
}

bool ESPRom::flashBegin(quint32 size, quint32 offset)
{
    quint32 numBlocks = (size + ESP_FLASH_BLOCK - 1) / ESP_FLASH_BLOCK,
            sectorsPerBlock = 16, sectorSize = 4096,
            numSectors = (size + sectorSize - 1) / sectorSize,
            startSector = offset / sectorSize,
            headSectors = sectorsPerBlock - (startSector % sectorsPerBlock);

    if (numSectors < headSectors)
        headSectors = numSectors;

    quint32 eraseSize;
    if (numSectors < 2 * headSectors)
        eraseSize = (numSectors + 1) / 2 * sectorSize;
    else
        eraseSize = (numSectors - headSectors) * sectorSize;

    char bytes[16];
    quint32toBytes(eraseSize, &bytes[0]);
    quint32toBytes(numBlocks, &bytes[4]);
    quint32toBytes(ESP_FLASH_BLOCK, &bytes[8]);
    quint32toBytes(offset, &bytes[12]);

    if(!sendCommand(FlashBegin, bytes, 16).isValid()){
        emit commandError("Failed to enter Flash download mode");
        return false;
    }

    return true;
}

bool ESPRom::flashBlock(const QByteArray &data, quint32 seq)
{
    char bytes[16];
    quint32toBytes(data.size(), &bytes[0]);
    quint32toBytes(seq, &bytes[4]);
    quint32toBytes(0, &bytes[8]);
    quint32toBytes(0, &bytes[12]);

    QByteArray packet;
    packet.append(bytes, 16);
    packet.append(data);

    if(!sendCommand(FlashData, packet, Tools::checksum(data)).isValid()){
        emit commandError("Failed to write to target Flash");
        return false;
    }

    return true;
}

bool ESPRom::flashFinish(bool reboot)
{
    char bytes[4];
    quint32toBytes((quint32)(!reboot), &bytes[0]);

    if(!sendCommand(FlashEnd, bytes, 4).isValid()){
        emit commandError("Failed to leave Flash mode");
        return false;
    }

    return true;
}

bool ESPRom::run(bool reboot)
{
    bool ret = false;
    if((ret = flashBegin(0, 0)))
        ret = flashFinish(reboot);

    return ret;
}

QByteArray ESPRom::readMAC()
{
    quint32 mac0 = readReg(ESP_OTP_MAC0);
    quint32 mac1 = readReg(ESP_OTP_MAC1);
    char mac[6] = {0, 0, 0, 0, 0, 0};

    if (((mac1 >> 16) & 0xff) == 0){
        mac[0] = 0x18;
        mac[1] = 0xfe;
        mac[2] = 0x34;
    } else if (((mac1 >> 16) & 0xff) == 1){
        mac[0] = 0xac;
        mac[1] = 0xd0;
        mac[2] = 0x74;
    }else
        return QByteArray();

    mac[3] = (mac1 >> 8) & 0xff;
    mac[4] = mac1 & 0xff;
    mac[5] = (mac0 >> 24) & 0xff;

    return QByteArray(mac, 6);
}

quint32 ESPRom::flashID()
{
    quint32 flashId = 0x0;
    if(flashBegin(0, 0))
        if(writeReg(0x60000240, 0x0, 0xffffffff))
            if(writeReg(0x60000200, 0x10000000, 0xffffffff))
                flashId = readReg(0x60000240);

    flashFinish(false);

    return flashId;
}

QByteArray ESPRom::flashRead(quint32 offset, quint32 size, quint32 count)
{
    char bytes[12];
    quint32toBytes(offset, &bytes[0]);
    quint32toBytes(size, &bytes[4]);
    quint32toBytes(count, &bytes[8]);

    QByteArray stub;
    stub.append(bytes, 12);
    stub.append(SFLASH_STUB, 60);

    if(!flashBegin(0, 0) ||
            !memBegin(stub.size(), 1, stub.size(), 0x40100000) ||
            !memBlock(stub, 0) ||
            !memFinish(0x4010001c)){
        return QByteArray();
    }

    int tries = 10;
    while(!waitForReadyRead(m_waitTimeout) && tries > 0){
        tries--;
    }

    emit commandStarted();

    QByteArray data;
    for(int i = 0; i < (int)count; i++){
        if(readBytes(1) != QByteArray("\xc0", 1)){
            emit commandError("Invalid head of packet (sflash read)");
            return QByteArray();
        }

        data += readAndEscape(size);
        emit flashReadProgress((100 * (i+1))/(float)count);

        if(readBytes(1) != QByteArray("\xc0", 1)){
            emit commandError("Invalid end of packet (sflash read)");
            return QByteArray();
        }
    }

    emit commandFinished();

    return data;
}

bool  ESPRom::flashUnlockDIO()
{
    bool ret = false;
    if((ret = flashBegin(0, 0)))
        if((ret = memBegin(0,0,0,0x40100000)))
            ret = memFinish(0x40000080);

    return ret;
}

bool ESPRom::flashErase()
{
    bool ret = false;
    if((ret = flashBegin(0, 0)))
        if((ret = memBegin(0,0,0,0x40100000)))
            ret = memFinish(0x40004984);

    return ret;
}

} //namespace ESPFlasher

