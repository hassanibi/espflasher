#include "mainwindow.h"
#include "tools.h"
#include <QApplication>
#include <QFontDatabase>
#include <QStyle>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>

struct ESPFlasherQuery {
    QString portName;
    QSerialPort::BaudRate baudRate;
    QString command;
};

enum CommandLineParseResult
{
    CommandLineOk,
    CommandLineError,
    CommandLineVersionRequested,
    CommandLineHelpRequested
};


CommandLineParseResult parseCommandLine(QCommandLineParser &parser, ESPFlasherQuery *query, QString *errorMessage)
{
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption portOption(QStringList() << "p" << "port", "Serial port device", "/dev/ttyUSB0");
    QCommandLineOption baudOption(QStringList() << "b" << "baud", "Serial port baud rate", "115200");

    parser.addOption(portOption);
    parser.addOption(baudOption);

    parser.addPositionalArgument("load-ram", "Download an image to RAM and execute.");
    parser.addPositionalArgument("dump-mem", "Dump arbitrary memory to disk.");
    parser.addPositionalArgument("read-mem", "Read arbitrary memory location.");
    parser.addPositionalArgument("write-mem", "Write to arbitrary memory location.");
    parser.addPositionalArgument("write-flash", "Write an image to flash.");
    parser.addPositionalArgument("run", "Run application code in flash.");
    parser.addPositionalArgument("image-info", "Dump headers from an application image.");
    parser.addPositionalArgument("make-image", "Create an application image from binary files.");
    parser.addPositionalArgument("elf2image", "Create an application image from ELF file.");
    parser.addPositionalArgument("read-mac", "Read MAC address from OTP ROM.");
    parser.addPositionalArgument("flash-id", "Read SPI flash manufacturer and device ID.");
    parser.addPositionalArgument("read-flash", "Read SPI flash content.");
    parser.addPositionalArgument("erase-flash", "Perform Chip Erase on SPI flash.");


    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    if (!parser.parse(QApplication::arguments())) {
        *errorMessage = parser.errorText();
        return CommandLineError;
    }

    if (parser.isSet(versionOption))
        return CommandLineVersionRequested;

    if (parser.isSet(helpOption))
        return CommandLineHelpRequested;

    if (parser.isSet(portOption)) {
        bool isValid = false;
        query->portName = parser.value(portOption);
        QList<QSerialPortInfo>	availablePorts = QSerialPortInfo::availablePorts();
        for(int i = 0; i < availablePorts.size(); i++){

            if(availablePorts.at(i).systemLocation() == query->portName){
                isValid = true;
                break;
            }
        }
        if(!isValid){
            *errorMessage = "Bad serial port name: " + query->portName;
            return CommandLineError;
        }

    }

    if (parser.isSet(baudOption)) {
        bool isValid = false;
        const qint32 baudParameter = parser.value(baudOption).toInt();
        static QList<qint32> standardBaudRates = QSerialPortInfo::standardBaudRates();
        for(int i = 0; i < standardBaudRates.size(); i++){
            if(standardBaudRates.at(i) == baudParameter){
                isValid = true;
                break;
            }
        }

        if(!isValid){
            *errorMessage = "Bad serial port name: " + query->portName;
            return CommandLineError;
        }
        query->baudRate = (QSerialPort::BaudRate)baudParameter;
    }

    const QStringList positionalArguments = parser.positionalArguments();
    if (positionalArguments.isEmpty()) {
        *errorMessage = "Argument 'command' missing.";
        return CommandLineOk;
    }

    if (positionalArguments.size() > 1) {
        *errorMessage = "Several 'command' arguments specified.";
        return CommandLineError;
    }

    query->command = positionalArguments.first();

    return CommandLineOk;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("espflasher");
    QApplication::setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("An open source and cross platform firmware programmer for ESP8266 chip");

    ESPFlasherQuery query;
    QString errorMessage;
    switch (parseCommandLine(parser, &query, &errorMessage)) {
    case CommandLineOk:
        break;
    case CommandLineError:
        fputs(qPrintable(errorMessage), stderr);
        fputs("\n\n", stderr);
        fputs(qPrintable(parser.helpText()), stderr);
        return 1;
    case CommandLineVersionRequested:
        printf("%s %s\n", qPrintable(QApplication::applicationName()),
               qPrintable(QApplication::applicationVersion()));
        return 0;
    case CommandLineHelpRequested:
        parser.showHelp();
        Q_UNREACHABLE();
    }


    QStringList imageCommands = QStringList() << "image-info" << "make-image" << "elf2image";
    if(imageCommands.contains(query.command))
    {
        //TODO:
        return 0;
    }
    else
    {
        QFontDatabase::addApplicationFont(":/fonts/code128.ttf");
        QApplication::setStyle(QLatin1String("fusion"));

        MainWindow w;
        w.show();

        return app.exec();
    }
}
