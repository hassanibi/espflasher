#include "espfirmwareimage.h"
#include "tools.h"

#include <QFile>
#include <QDataStream>
#include <QDebug>

namespace ESPFlasher {

ESPFirmwareImage::ESPFirmwareImage(const QString &filename):
    m_error(NoError),
    m_flashMode(0),
    m_flashSizeFreq(0),
    m_entryPoint(0),
    m_checksum(0)
{

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)){
        m_error  = FileError;
        m_errorText = file.errorString();
        return;
    }

    QDataStream dataStream(&file);
    dataStream.setByteOrder(QDataStream::LittleEndian);

    int bytesCount = 0;
    quint8 magic;
    quint8 segments;

    dataStream >> magic >> segments >> m_flashMode >> m_flashSizeFreq >> m_entryPoint;
    bytesCount += 8;

    if(magic != ESP_IMAGE_MAGIC || segments > 16){
        m_error = InvalidFirmwareImage;
        m_errorText = "Invalid firmware image";
        return;
    }

    for(int i = 0; i < segments; i++){
        quint32 offset, size;
        dataStream >> offset >> size;
        bytesCount += 8;
        if (offset > 0x40200000 || offset < 0x3ffe0000 || size > 65536){
            m_error = SuspiciousSegmentLength;
            m_errorText = QString::asprintf("Suspicious segment 0x%x, length %d", offset, size);
            return;
        }
        int len;
        char data[size];
        if((len = (quint32)dataStream.readRawData(data, size)) < size){
            m_error = BadEndOfFile;
            m_errorText = QString::asprintf("End of file reading segment 0x%x, length %d (actual length %d)", offset, size, len);
            return;
        }
        bytesCount += size;

        Segment segment;
        segment.offset = offset;
        segment.size = size;
        segment.data = QByteArray(data, size);
        m_segments.append(segment);
    }

    dataStream.skipRawData(15 - (bytesCount % 16));
    dataStream >> m_checksum;

    file.close();
}


void ESPFirmwareImage::addSegment(quint32 addr, QByteArray data)
{
    if (data.size() % 4){
        for(int i = 0; i < (4 - data.size() % 4); i++){
            data += "\x00";
        }
    }

    if (data.size() > 0){
        Segment segment;
        segment.offset = addr;
        segment.size = data.size();
        segment.data = data.data();
        m_segments.append(segment);
    }

}

void ESPFirmwareImage::save(const QString &filename)
{
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly)){
        QDataStream dataStream(&file);
        dataStream.setByteOrder(QDataStream::LittleEndian);

        int writeCount = 0;

        dataStream << (quint8)ESP_IMAGE_MAGIC << (quint8)m_segments.size() <<  m_flashMode << m_flashSizeFreq << m_entryPoint;
        writeCount += 8;

        quint8 checksum = (quint8)ESP_CHECKSUM_MAGIC;

        for(int i = 0; i < m_segments.size(); i++){
            dataStream << m_segments.at(i).offset << m_segments.at(i).size;
            writeCount += 8;
            writeCount += dataStream.writeRawData(m_segments.at(i).data.data(), m_segments.at(i).data.size());
            checksum = Tools::checksum(m_segments.at(i).data, checksum);
        }

        for(int i = 0; i < 15 - (writeCount % 16); i++){
            dataStream.writeRawData("\x00", 1);
        }

        dataStream << checksum;

        file.close();
    }
}

} //namespace ESPFlasher

