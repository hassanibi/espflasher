#ifndef ESPFIRMWAREIMAGE_H
#define ESPFIRMWAREIMAGE_H

#include <QString>
#include <QList>

namespace ESPFlasher {

struct Segment {
    int offset;
    int size;
    QByteArray data;
};


class ESPFirmwareImage
{
public:
    ESPFirmwareImage(const QString &filename = QString());

    enum ImageError {
        NoError,
        FileError,
        InvalidFirmwareImage,
        SuspiciousSegmentLength,
        BadEndOfFile,

        IMAGE_ERROR_COUNT
    };

    void addSegment(quint32 addr, QByteArray data);
    bool save(const QString &filename);

    void setEntryPoint(quint32 entryPoint) { m_entryPoint = entryPoint; }

    bool isValid() { return m_error == NoError; }
    QString errorText() { return m_errorText; }

    QList<Segment> segments() { return m_segments; }
    quint32 entryPoint() { return m_entryPoint; }
    quint8 flashMode() { return m_flashMode; }
    quint8 flashSizeFreq() { return m_flashSizeFreq; }
    quint8 checksum() { return m_checksum; }

private:
    QList<Segment> m_segments;
    quint32 m_entryPoint;
    quint8 m_flashMode;
    quint8 m_flashSizeFreq;
    quint8 m_checksum;
    ImageError m_error;
    QString m_errorText;
};

} //namespace ESPFlasher

#endif // ESPFIRMWAREIMAGE_H
