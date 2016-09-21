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



class ESPRom : public QSerialPort
{
    Q_OBJECT
public:
    explicit ESPRom(QObject *parent = 0);
    explicit ESPRom(const QString &portName, const QSerialPort::BaudRate baudRate = QSerialPort::Baud115200, int resetMode = 1, QObject *parent = 0);
    ~ESPRom();

    enum ResetMode {
        None = 1,
        Auto,
        CK,
        Wifio,
        NodeMCU,
        DTROnly
    };

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
        setPortName(portName);
        setBaudRate(baudRate);
    }

    void setResetMode(int resetMode) { m_resetMode = resetMode; }

    CommandResponse sendCommand(ESPCommand cmd, const char *data, quint16 size, quint32 chk = 0);
    CommandResponse sendCommand(ESPCommand cmd = NoCommand, const QByteArray &data = QByteArray(), quint32 chk = 0);
    CommandResponse receiveResponse();

    bool openPort();
    bool isPortOpen() const { return isOpen() && m_isSync; }
    void closePort();

    QString macAddress() {
        if(m_macAddress.isEmpty())
            m_macAddress = readMAC().toHex();
        return m_macAddress;
    }

    QString deviceID() {
        if(m_flashID == 0)
            m_flashID = flashID();
        return QString("%1%2").arg((m_flashID >> 8) & 0xff, 1, 16)
                .arg((m_flashID >> 16) & 0xff, 1, 16).toUpper();
    }

    QString deviceManufacturer() {
        if(m_flashID == 0)
            m_flashID = flashID();
        return QString("%1").arg(m_flashID & 0xff, 1, 16).toUpper();
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

protected:
    virtual qint64	readData(char * data, qint64 maxSize);
    virtual qint64	writeData(const char * data, qint64 maxSize);

signals:
    void commandStarted(ESPCommand cmd = NoCommand);
    void commandFinished(ESPCommand cmd = NoCommand);
    void commandError(const QString &errorText);
    void flashReadProgress(int progress);

private slots:
    void handleSerialError(QSerialPort::SerialPortError error);

private:
    void resetDevice(int mode = Auto);
    bool sync();
    QByteArray readAndEscape(int size = 1);
    QByteArray readBytes(int size = 1);
    void writeToPort(QByteArray data);
    QString errorText(CommandResponse response);

private:
    QString m_macAddress;
    int m_waitTimeout;
    bool m_isSync;
    quint32 m_flashID;
    int m_resetMode;
};

} //namespace ESPFlasher

#endif // ESPROM_H
