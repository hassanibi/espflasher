#ifndef TOOLS_H
#define TOOLS_H

#include <QByteArray>
#include <QPointer>


namespace ESPFlasher {

// Maximum block sized for RAM and Flash writes, respectively.
#define ESP_RAM_BLOCK       0x1800
#define ESP_FLASH_BLOCK     0x400

// Default baudrate. The ROM auto-bauds, so we can use more or less whatever we want.
#define ESP_ROM_BAUD        115200

// First byte of the application image
#define ESP_IMAGE_MAGIC     0xe9

// Initial state for the checksum routine
#define ESP_CHECKSUM_MAGIC  0xef

// OTP ROM addresses
#define ESP_OTP_MAC0        0x3ff00050
#define ESP_OTP_MAC1        0x3ff00054

// Sflash stub: an assembly routine to read from spi flash and send to host
#define SFLASH_STUB     "\x80\x3c\x00\x40\x1c\x4b\x00\x40\x21\x11\x00\x40\x00\x80" \
    "\xfe\x3f\xc1\xfb\xff\xd1\xf8\xff\x2d\x0d\x31\xfd\xff\x41\xf7\xff\x4a" \
    "\xdd\x51\xf9\xff\xc0\x05\x00\x21\xf9\xff\x31\xf3\xff\x41\xf5\xff\xc0" \
    "\x04\x00\x0b\xcc\x56\xec\xfd\x06\xff\xff\x00\x00"

static inline void quint32toBytes(quint32 num, char *data)
{
    unsigned char *udata = (unsigned char *)data;
    udata[0] = (num & 0xff);
    udata[1] = (num & 0xff00) >> 8;
    udata[2] = (num & 0xff0000) >> 16;
    udata[3] = (num & 0xff000000) >> 24;
}

static inline void quint16toBytes(quint16 num, char *data)
{
    unsigned char *udata = (unsigned char *)data;
    udata[0] = (num & 0xff);
    udata[1] = (num & 0xff00) >> 8;
}

static inline void quint8toBytes(quint8 num, char *data)
{
    unsigned char *udata = (unsigned char *)data;
    udata[0] = (num & 0xff);
}

static inline quint32 bytes2quint32(const char *data)
{
    const unsigned char *udata = (const unsigned char *)data;
    return (quint32(udata[3]) << 24)
            | (quint32(udata[2]) << 16)
            | (quint32(udata[1]) << 8)
            | (quint32(udata[0]));
}

static inline quint16 bytes2quint16(const char *data)
{
    const unsigned char *udata = (const unsigned char *)data;
    return (quint16(udata[1]) << 8)
            | (quint16(udata[0]));
}

enum OsType { OsTypeWindows, OsTypeLinux, OsTypeMac, OsTypeOtherUnix, OsTypeOther };

class Tools
{
public:
    Tools();

    static inline OsType hostOs();

    static bool isWindowsHost() { return hostOs() == OsTypeWindows; }
    static bool isLinuxHost() { return hostOs() == OsTypeLinux; }
    static bool isMacHost() { return hostOs() == OsTypeMac; }


    static quint8 checksum(const QByteArray &data, quint8 state = ESP_CHECKSUM_MAGIC)
    {
        for(int i = 0; i < data.size(); i++)
            state ^= data.at(i);
        return state;
    }

    static quint32 divRoundup(quint32 a, quint32 b)
    {
        return (int(a) + int(b) - 1) / int(b);
    }

    template<typename DialogT, typename... ArgsT>
    static int
    openDialog (QPointer<DialogT>& dialog, ArgsT&&... args)
    {
        if (dialog.isNull ())
        {
            dialog = new DialogT (std::forward<ArgsT> (args)...);
            dialog->setAttribute (Qt::WA_DeleteOnClose);
        }
        else
        {
            dialog->raise ();
            dialog->activateWindow ();
        }
        return dialog->exec();
    }

    static QString compilerString()
    {
#if defined(Q_CC_CLANG) // must be before GNU, because clang claims to be GNU too
        QString isAppleString;
#if defined(__apple_build_version__) // Apple clang has other version numbers
        isAppleString = QLatin1String(" (Apple)");
#endif
        return QLatin1String("Clang " ) + QString::number(__clang_major__) + QLatin1Char('.')
                + QString::number(__clang_minor__) + isAppleString;
#elif defined(Q_CC_GNU)
        return QLatin1String("GCC " ) + QLatin1String(__VERSION__);
#elif defined(Q_CC_MSVC)
        if (_MSC_VER >= 1800) // 1800: MSVC 2013 (yearly release cycle)
            return QLatin1String("MSVC ") + QString::number(2008 + ((_MSC_VER / 100) - 13));
        if (_MSC_VER >= 1500) // 1500: MSVC 2008, 1600: MSVC 2010, ... (2-year release cycle)
            return QLatin1String("MSVC ") + QString::number(2008 + 2 * ((_MSC_VER / 100) - 15));
#endif
        return QLatin1String("<unknown compiler>");
    }

    static QString versionString()
    {
        return QObject::tr("ESP Flasher %1%2").arg(QLatin1String("1.0.0")).arg("");
    }

    static QString buildCompatibilityString()
    {
        return QObject::tr("Based on Qt %1 (%2, %3 bit)").arg(QLatin1String(qVersion()),
                                                              compilerString(),
                                                              QString::number(QSysInfo::WordSize));
    }

};

OsType Tools::hostOs()
{
#if defined(Q_OS_WIN)
    return OsTypeWindows;
#elif defined(Q_OS_LINUX)
    return OsTypeLinux;
#elif defined(Q_OS_MAC)
    return OsTypeMac;
#elif defined(Q_OS_UNIX)
    return OsTypeOtherUnix;
#else
    return OsTypeOther;
#endif
}

} //namespace ESPFlasher

#endif // TOOLS_H
