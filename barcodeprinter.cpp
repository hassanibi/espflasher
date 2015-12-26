#include "barcodeprinter.h"
#include <QPrinter>
#include <QFont>
#include <QFileDialog>

BarcodePrinter::BarcodePrinter(const QString &fileName, QObject *parent) :
    QObject(parent)
{
    m_printer.setOutputFileName(fileName);
    m_printer.setOutputFormat(QPrinter::PdfFormat);
    m_printer.setColorMode(QPrinter::GrayScale);
    m_printer.setPageSizeMM(QSizeF(65,22));
    m_printer.setPaperSize(QSizeF(65,22), QPrinter::Millimeter);
    m_printer.setResolution(203);
    m_printer.setPageMargins(0,0,0,0, QPrinter::Millimeter);
    m_printer.setOrientation(QPrinter::Portrait);
}

void BarcodePrinter::printBarcode(QString barcodeText)
{  

    double MmToDot = 8; //Printer DPI = 203 => 8 dots per mm
    QPainter painter(&m_printer);

    QRect barcodeRect = QRect(2*MmToDot,2*MmToDot,60.5*MmToDot,10*MmToDot);
    QRect barcodeTextRect = QRect(2*MmToDot,14.5*MmToDot,60.5*MmToDot,5*MmToDot);

    QFont barcodefont = QFont("Code 128", 48, QFont::Normal);
    barcodefont.setLetterSpacing(QFont::AbsoluteSpacing,0.0);
    painter.setFont(barcodefont);

    QString arr = encodeBarcode(barcodeText);
    painter.drawText(barcodeRect, Qt::AlignCenter, arr);

    painter.setFont(QFont("PT Sans", 12));
    painter.drawText(barcodeTextRect, Qt::AlignCenter, barcodeText);

    painter.end();
}

QString BarcodePrinter::encodeBarcode(QString code)
{
    QString encoded;

    encoded.prepend(QChar(codeToChar(CODE128_B_START))); //Start set with B Code 104
    encoded.append(code);
    encoded.append(QChar(calculateCheckCharacter(code)));
    encoded.append(QChar(codeToChar(CODE128_STOP))); //End set with Stop Code 106

    return encoded;
}

int BarcodePrinter::calculateCheckCharacter(QString code)
{
    QByteArray encapBarcode(code.toUtf8()); //Convert code to utf8

    //Calculate check character
    long long sum = CODE128_B_START; //The sum starts with the B Code start character value
    int weight = 1; //Initial weight is 1

    foreach(char ch, encapBarcode) {
        int code_char = charToCode((int)ch); //Calculate character code
        sum += code_char*weight; //add weighted code to sum
        weight++; //increment weight
    }

    int remain = sum%103; //The check character is the modulo 103 of the sum

    //Calculate the font integer from the code integer
    if(remain >= 95)
        remain += 105;
    else
        remain += 32;

    return remain;
}

int BarcodePrinter::codeToChar(int code)
{
    return code + 105;
}

int BarcodePrinter::charToCode(int ch)
{
    return ch - 32;
}
