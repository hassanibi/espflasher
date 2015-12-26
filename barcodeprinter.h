#ifndef BARCODEPRINTER_H
#define BARCODEPRINTER_H

#include <QObject>
#include <QtPrintSupport>

#define CODE128_B_START 104
#define CODE128_STOP 106

class BarcodePrinter : public QObject
{
    Q_OBJECT
public:
    explicit BarcodePrinter(const QString &fileName, QObject *parent = 0);

    void printBarcode(QString barcodeText);

private:

    QString encodeBarcode(QString code);
    int calculateCheckCharacter(QString code);

    int codeToChar(int code);
    int charToCode(int ch);

    QPrinter m_printer;

};

#endif // BARCODEPRINTER_H


