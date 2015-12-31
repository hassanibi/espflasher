#ifndef ESPROM_H
#define ESPROM_H

/*
 * Based on https://github.com/themadinventor/esptool.git
 */

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <QDataStream>

namespace ESPFlasher {

class CommandResponse {
public:
    enum ResponseError{
        ResponseOK = 0x0,
        InvalidResponse = 0x1,
        InvalidPacketHead = 0x2,
        InvalidPacketEnd = 0x3

    };

    CommandResponse(ResponseError error = ResponseOK){ m_error = error; }

    bool isValid() {
        return error() == ResponseOK && body == QByteArray("\x00\x00", 2);
    }

    ResponseError error(){ return m_error; }
    void setError(ResponseError error) { m_error = error;}

    quint8 cmd;
    quint16 size;
    quint32 value;
    QByteArray body;


private:
    ResponseError m_error;

};



class ESPRom : public QObject
{
    Q_OBJECT
public:
    explicit ESPRom(QObject *parent = 0);
    explicit ESPRom(const QString &portName, const QSerialPort::BaudRate baudRate = QSerialPort::Baud115200, QObject *parent = 0);
    ~ESPRom();

    enum ESPCommand {
        NoCommand = 0x00,
        FlashBegin = 0x02,
        FlashData = 0x03,
        FlashEnd = 0x04,
        MemBegin = 0x05,
        MemEnd = 0x06,
        MemData = 0x07,
        Sync = 0x08,
        WriteReg = 0x09,
        ReadReg = 0x0a
    };

    void setSerialPort(const QString &portName, const QSerialPort::BaudRate baudRate = QSerialPort::Baud115200){
        m_serialPort->setPortName(portName);
        m_serialPort->setBaudRate(baudRate);
    }

    QString portName() { return m_serialPort->portName(); }

    CommandResponse sendCommand(ESPCommand cmd, const char *data, quint16 size, quint32 chk = 0);
    CommandResponse sendCommand(ESPCommand cmd = NoCommand, const QByteArray &data = QByteArray(), quint32 chk = 0);
    CommandResponse receiveResponse();

    bool open();
    bool isOpen() { return m_serialPort->isOpen(); }
    void close();

    QString macAddress() {
        if(m_macAddress.isEmpty())
            m_macAddress = readMAC().toHex();
        return m_macAddress;
    }

    quint32 readReg(quint32 addr);
    bool writeReg(quint32 addr, quint32 value, quint32 mask, quint32 delayus = 0);

    bool memBegin(quint32 size, quint32 blocks, quint32 blocksize, quint32 offset);
    bool memBlock(const QByteArray &data, quint32 seq);
    bool memFinish(quint32 entrypoint = 0);

    bool flashBegin(quint32 size, quint32 offset);
    bool flashBlock(const QByteArray &data, quint32 seq);
    bool flashFinish(bool reboot = false);

    bool run(bool reboot = false);
    QByteArray readMAC();
    quint32 flashID();
    QByteArray flashRead(quint32 offset, quint32 size, quint32 count = 1);

    bool flashUnlockDIO();
    bool flashErase();

signals:
    void commandStarted(ESPCommand cmd = NoCommand);
    void commandFinished(ESPCommand cmd = NoCommand);
    void commandError(const QString &errorText);

private slots:
    void readData();
    void handleSerialError(QSerialPort::SerialPortError error);

private:
    bool sync();
    QByteArray readAndEscape(int size = 1);
    QByteArray readBytes(int size = 1);
    void write(QByteArray data);
    QString errorText(CommandResponse response);

private:
    QSerialPort *m_serialPort;
    QString m_macAddress;
    int m_waitTimeout;
};

} //namespace ESPFlasher

#endif // ESPROM_H
