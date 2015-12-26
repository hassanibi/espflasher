#include "esprom.h"
#include "tools.h"

#include <QThread>
#include <QDataStream>
#include <QDebug>

namespace ESPFlasher {

ESPRom::ESPRom(QObject *parent):
    QObject(parent),
    serialPort(new QSerialPort(this))
{

}

ESPRom::ESPRom(const QString &portName, const QSerialPort::BaudRate baudRate, QObject *parent) :
    QObject(parent)
{
    serialPort = new QSerialPort(this);
    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);

    connect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleSerialError(QSerialPort::SerialPortError)));


}

ESPRom::~ESPRom()
{
    if (serialPort->isOpen())
        serialPort->close();

    delete serialPort;
}

void ESPRom::readData()
{

}

void ESPRom::writeData(const QByteArray &data)
{
    qInfo(qPrintable("Serial Data sent %d..."), data.size());
    serialPort->write(data);
}

QByteArray ESPRom::read(int size)
{
    qint64 readCount = 0;
    QByteArray data;
    while(data.size() < size){
        char buffer[1];
        readCount += serialPort->read(buffer, 1);
        if (buffer[0] == '\xdb'){
            readCount += serialPort->read(buffer, 1);
            if (buffer[0] == '\xdc')
                data.append('\xc0');
            else if (buffer[0] == '\xdd')
                data.append('\xdb');
            else{
                //Invalid SLIP escape
                return data;
            }
        }else{
            data.append(buffer, 1);
        }
    }

    return data;
}

void ESPRom::write(QByteArray data)
{
    QByteArray buf = "\xc0" + (data.replace("\xdb","\xdb\xdd").replace("\xc0","\xdb\xdc")) + "\xc0";

    serialPort->write(buf.data(), buf.size());
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

        write(buffer);

        serialPort->waitForReadyRead(5000);
    }

    int retries = 100;
    while (retries > 0){
        CommandResponse response = receiveResponse();
        if(cmd == NoCommand || response.cmd == (quint8)cmd){
            emit commandFinished(cmd);
            return response;
        }
        retries--;
    }

    emit commandFinished(cmd);

    return CommandResponse::InvalidResponse;
}

CommandResponse ESPRom::sendCommand(ESPCommand cmd, const QByteArray &data, quint32 chk)
{
    return sendCommand(cmd, data.data(), data.size(), chk);
}

CommandResponse ESPRom::receiveResponse()
{
    if(read(1) != QByteArray("\xc0", 1)){
        return CommandResponse::InvalidPacketHead;
    }

    QByteArray bytes = read(8);
    if(bytes.at(0) != 0x01){
        return CommandResponse::InvalidResponse;
    }

    CommandResponse response;
    response.cmd = bytes.at(1);
    response.size = bytes2quint16(&bytes.data()[2]);
    response.value = bytes2quint32(&bytes.data()[4]);
    response.body = read(response.size);

    if(read(1) != QByteArray("\xc0", 1)){
        return CommandResponse::InvalidPacketEnd;
    }

    return response;
}

bool ESPRom::sync()
{
    QByteArray packet("\x07\x07\x12\x20");
    for(int i = 0; i < 32; i++){
        packet.append("\x55", 1);
    }

    if(!sendCommand(Sync, packet).isValid()){
        serialPort->close();
        return false;
    }

    for(int i = 0; i < 8; i++){
        sendCommand();
    }

    return true;
}

bool ESPRom::open()
{
    if(serialPort->isOpen())
        return true;

    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (serialPort->open(QIODevice::ReadWrite)) {
        return sync();
    }

    return false;
}


void ESPRom::close()
{
    if(serialPort->isOpen()){
        serialPort->close();
    }
}

void ESPRom::handleSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        if (serialPort->isOpen())
            serialPort->close();

    }

    if(error != QSerialPort::NoError){
        qInfo(qPrintable("Serial error %d..."), error);
    }
}

quint32 ESPRom::readReg(quint32 addr)
{
    char bytes[4];
    quint32toBytes(addr, bytes);

    CommandResponse response = sendCommand(ReadReg, bytes, 4);

    if(!response.isValid()){
        return -1;
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

    return sendCommand(WriteReg, bytes, 16).isValid();
}

bool ESPRom::memBegin(quint32 size, quint32 blocks, quint32 blocksize, quint32 offset)
{
    char bytes[16];
    quint32toBytes(size, &bytes[0]);
    quint32toBytes(blocks, &bytes[4]);
    quint32toBytes(blocksize, &bytes[8]);
    quint32toBytes(offset, &bytes[12]);

    return sendCommand(MemBegin, bytes, 16).isValid();
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

    return sendCommand(MemData, packet).isValid();
}

bool ESPRom::memFinish(quint32 entrypoint)
{
    char bytes[8];
    quint32toBytes((quint32)(entrypoint == 0), &bytes[0]);
    quint32toBytes(entrypoint, &bytes[4]);

    return sendCommand(MemEnd, bytes, 8).isValid();
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

    return sendCommand(FlashBegin, bytes, 16).isValid();
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

    return sendCommand(FlashData, packet, Tools::checksum(data)).isValid();
}

bool ESPRom::flashFinish(bool reboot)
{
    char bytes[4];
    quint32toBytes((quint32)(!reboot), &bytes[0]);

    return sendCommand(FlashEnd, bytes, 4).isValid();
}

void ESPRom::run(bool reboot)
{
    flashBegin(0, 0);
    flashFinish(reboot);
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
    flashBegin(0, 0);
    writeReg(0x60000240, 0x0, 0xffffffff);
    writeReg(0x60000200, 0x10000000, 0xffffffff);
    quint32 flashId = readReg(0x60000240);
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

    flashBegin(0, 0);
    memBegin(stub.size(), 1, stub.size(), 0x40100000);
    memBlock(stub, 0);
    memFinish(0x4010001c);

    QByteArray data;
    for(int i = 0; i < (int)count; i++){
        if(read(1) != QByteArray("\xc0", 1)){
            return QByteArray();
        }

        data += read(size);

        if(read(1) != QByteArray("\xc0", 1)){
            return QByteArray();
        }
    }

    return data;
}

void ESPRom::flashUnlockDIO()
{
    flashBegin(0, 0);
    memBegin(0,0,0,0x40100000);
    memFinish(0x40000080);

}

void ESPRom::flashErase()
{
    flashBegin(0, 0);
    memBegin(0,0,0,0x40100000);
    memFinish(0x40004984);
}

} //namespace ESPFlasher

