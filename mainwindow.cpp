#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "constants.h"
#include "esprom.h"
#include "espfirmwareimage.h"
#include "tools.h"
#include "imagechooser.h"
#include "flashinputdialog.h"
#include "loglist.h"
#include "barcodeprinter.h"
#include "makeimagedialog.h"
#include "versiondialog.h"
#include "preferencesdialog.h"

#ifdef WITH_POPPLER_QT5
#include <poppler/qt5/poppler-qt5.h>
#endif

#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QLineEdit>
#include <QToolButton>
#include <QCheckBox>
#include <QLabel>
#include <QFileDialog>
#include <QFile>
#include <QPrintDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDesktopServices>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_esp(new ESPFlasher::ESPRom),
    m_inputDialog(),
    m_makeImageDialog(),
    m_aboutDialog(),
    m_currentAction(NoAction)
{
    ui->setupUi(this);

    QSettings settings;
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    restoreState(settings.value("mainWindowState").toByteArray());

    fillComboBoxes();

    connect(m_esp, SIGNAL(commandStarted(ESPCommand)), this, SLOT(espCmdStarted()));
    connect(m_esp, SIGNAL(commandFinished(ESPCommand)), this, SLOT(espCmdFinished()));
    connect(m_esp, SIGNAL(commandError(QString)), this, SLOT(espError(QString)));
    connect(ui->actionImport_image_file_list, SIGNAL(triggered(bool)), this, SLOT(importImageList()));
    connect(ui->actionExport_image_file_list, SIGNAL(triggered(bool)), this, SLOT(exportImageList()));
    connect(ui->openBtn, SIGNAL(clicked(bool)), this, SLOT(open()));
    connect(ui->writeFlashBtn, SIGNAL(clicked(bool)), SLOT(writeFlash()));
    connect(ui->readFlashBtn, SIGNAL(clicked(bool)), SLOT(readFlash()));
    connect(ui->eraseFlashBtn, SIGNAL(clicked(bool)), this, SLOT(eraseFlash()));
    connect(ui->loadRamBtn, SIGNAL(clicked(bool)), SLOT(loadRam()));
    connect(ui->dumpMemoryBtn, SIGNAL(clicked(bool)), SLOT(dumpMemory()));
    connect(ui->readMemoryBtn, SIGNAL(clicked(bool)), SLOT(readMemory()));
    connect(ui->writeMemoryBtn, SIGNAL(clicked(bool)), SLOT(writeMemory()));
    connect(ui->makeImageBtn, SIGNAL(clicked(bool)), this, SLOT(makeImage()));
    connect(ui->runImageBtn, SIGNAL(clicked(bool)), this, SLOT(runImage()));
    connect(ui->copyMacBtn, SIGNAL(clicked(bool)), SLOT(copyMAC()));
    connect(ui->actionPreferences, SIGNAL(triggered(bool)), this, SLOT(openPreferences()));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(openAbout()));
    connect(ui->actionExit, SIGNAL (triggered ()), qApp, SLOT (quit ()));


#ifdef WITH_POPPLER_QT5
    connect(ui->printMacBtn, SIGNAL(clicked(bool)), SLOT(printMAC()));
#else
    ui->printMacBtn->setVisible(false);
#endif

    for(int i = 0; i < 4; i++){
        addFileField();
    }

    enableActions();

    m_scanTimer = new QTimer(this);
    connect(m_scanTimer,SIGNAL(timeout()),this,SLOT(scanSerialPorts()));
    m_scanTimer->start(1000);
}

MainWindow::~MainWindow()
{
    delete m_esp;
    delete m_scanTimer;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());

    settings.setValue("baudRate", ui->baudRate->currentIndex());
    settings.setValue("resetMode", ui->resetMode->currentIndex());
    settings.setValue("spiMode", ui->spiMode->currentIndex());
    settings.setValue("spiSpeed", ui->spiSpeed->currentIndex());
    settings.setValue("flashSize", ui->flashSize->currentIndex());


    event->accept();
}


void MainWindow::openPreferences()
{
    ESPFlasher::Tools::openDialog (m_preferencesDialog, this);
}

void MainWindow::openAbout()
{
    ESPFlasher::Tools::openDialog (m_aboutDialog, this);
}

void MainWindow::addFileField()
{
    ImageChooser *fileField = new ImageChooser(false, ui->filesTab);
    m_filesFields.append(fileField);
    ui->verticalLayoutFT->addWidget(fileField);
}

void MainWindow::scanSerialPorts()
{
    int index = ui->serialPort->currentIndex();
    QString serialPort = ui->serialPort->currentData().toString();
    bool disconnected = true;

    ui->serialPort->clear();

    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    foreach (const QSerialPortInfo &info, availablePorts) {
        ui->serialPort->addItem(info.portName(), info.systemLocation());
        if(info.systemLocation() == serialPort){
            disconnected = false;
        }
    }

    if(disconnected && m_esp->isPortOpen()){
        m_esp->closePort();
        enableActions();
        ui->logList->addEntry(tr("Disconnected from ESP8266."), LogList::Warning);
        return;
    }

    ui->serialPort->setCurrentIndex(index > -1  ?index : 0);
}

void MainWindow::fillComboBoxes()
{
    QSettings settings;

    QList<QSerialPortInfo>	availablePorts = QSerialPortInfo::availablePorts();
    for(int i = 0; i < availablePorts.size(); i++){
        ui->serialPort->addItem(availablePorts.at(i).portName(), availablePorts.at(i).systemLocation());
    }
    ui->openBtn->setEnabled(!availablePorts.isEmpty());

    static QList<qint32> standardBaudRates = QSerialPortInfo::standardBaudRates();
    for(int i = 0; i < standardBaudRates.size(); i++){
        if(standardBaudRates.at(i) > 9599 && standardBaudRates.at(i) < 115201)
            ui->baudRate->addItem(QString::number(standardBaudRates.at(i)), standardBaudRates.at(i));
    }
    ui->baudRate->setCurrentIndex(settings.value("baudRate", standardBaudRates.size() - 1).toInt());

    QList<QString> resetModes;
    QList<int> resetModesData;
    resetModes << "Auto" << "CK" << "Wifio" << "NodeMCU" << "DTROnly";
    resetModesData << 1 << 2 << 3 << 4 << 5;
    for(int i = 0; i < resetModes.size(); i++){
        ui->resetMode->addItem(resetModes.at(i), resetModesData.at(i));
    }
    ui->resetMode->setCurrentIndex(settings.value("resetMode", 0).toInt());

    QList<QString> modes;
    QList<int> modesData;
    modes << "QIO (default)" << "QOUT" << "DIO" << "DOUT";
    modesData << 0 << 1 << 2 << 3;
    for(int i = 0; i < modes.size(); i++){
        ui->spiMode->addItem(modes.at(i), modesData.at(i));
    }

    ui->spiMode->setCurrentIndex(settings.value("spiMode", 0).toInt());

    QList<QString> sizes;
    QList<int> sizesData;
    sizes << "2 MBit" << "4 MBit (default)" << "8 MBit" << "16 MBit" << "32 MBit";
    sizesData << 0x10 << 0x00 << 0x20 << 0x30 << 0x40;
    for(int i = 0; i < sizes.size(); i++){
        ui->flashSize->addItem(sizes.at(i), sizesData.at(i));
    }
    ui->flashSize->setCurrentIndex(settings.value("flashSize", 1).toInt());

    QList<QString> speeds;
    QList<int> speedsData;
    speeds << "20 Mhz" << "26 Mhz" << "40 Mhz (default)" << "80 Mhz";
    speedsData << 2 << 1 << 0 << 0xf;
    for(int i = 0; i < speeds.size(); i++){
        ui->spiSpeed->addItem(speeds.at(i), speedsData.at(i));
    }
    ui->spiSpeed->setCurrentIndex(settings.value("spiSpeed", 2).toInt());

}

void MainWindow::enableActions()
{
    bool deviceConnected = m_esp->isPortOpen();
    ui->writeFlashBtn->setEnabled(deviceConnected);
    ui->readFlashBtn->setEnabled(deviceConnected);
    ui->readMemoryBtn->setEnabled(deviceConnected);
    ui->writeMemoryBtn->setEnabled(deviceConnected);
    ui->dumpMemoryBtn->setEnabled(deviceConnected);
    ui->runImageBtn->setEnabled(deviceConnected);
    ui->loadRamBtn->setEnabled(deviceConnected);
    ui->eraseFlashBtn->setEnabled(deviceConnected);

    ui->macAddressGroup->setEnabled(deviceConnected);
#ifdef WITH_POPPLER_QT5
    ui->printMacBtn->setVisible(deviceConnected);
#endif
    ui->copyMacBtn->setVisible(deviceConnected);
    ui->macBarcodeLabel->setVisible(deviceConnected);

    ui->openBtn->setText(deviceConnected ? tr("Disconnect") : tr("Connect"));
}

void MainWindow::open()
{
    if(m_esp->isPortOpen()){
        m_esp->closePort();
        enableActions();
        ui->logList->addEntry(tr("Disconnected from ESP8266."), LogList::Warning);
        return;
    }

    m_esp->setSerialPort(ui->serialPort->currentData().toString(), (QSerialPort::BaudRate)ui->baudRate->currentData().toInt());
    m_esp->setResetMode(ui->resetMode->currentData().toInt());

    ui->openBtn->setEnabled(false);
    setCursor(Qt::WaitCursor);

    ui->openBtn->setText(tr("Connecting..."));
    ui->logList->addEntry(tr("Connecting..."));

    QApplication::processEvents();

    int tries = 0;
    while (tries++ < 3) {
        if(m_esp->openPort()){
            ui->logList->addEntry(tr("Connected to ESP8266 on %1 (%2 attempts)").arg(m_esp->portName()).arg(tries));
            displayMAC();
            break;
        }
        QCoreApplication::processEvents();
    }

    ui->openBtn->setEnabled(true);
    setCursor(Qt::ArrowCursor);
    enableActions();

    if(!m_esp->isPortOpen()){
        QString msg = "Failed to connect to ESP8266\n" \
                      "Make sure that GPIO0 and GPIO15 are connected to low-level(ground), GPIO2 and CH_PD are connected\n"\
                      "to high-level(power source) And reboot device(reconnect power or connect RST pin to ground for a second).";
        ui->logList->addEntry(msg, LogList::Error);
    }
}

void MainWindow::displayMAC()
{
    if(!m_esp->isPortOpen()){
        return;
    }

    QString macAddress =  m_esp->macAddress().toUpper();
#ifdef WITH_POPPLER_QT5
    QString filename(macAddress +".pdf");
    BarcodePrinter *printer = new BarcodePrinter(filename);
    printer->printBarcode(macAddress);

    delete printer;

    if(!QFileInfo(filename).exists()){
        return;
    }

    Poppler::Document* document = Poppler::Document::load(filename);
    if (document && !document->isLocked()) {
        Poppler::Page* pdfPage = document->page(0);
        if (pdfPage) {
            QImage image = pdfPage->renderToImage(300.0, 300.0);
            if (!image.isNull()) {
                ui->macBarcodeLabel->setPixmap(QPixmap::fromImage(image).scaledToHeight(70));
            }
            delete pdfPage;
        }
        delete document;
    }
#else
    ui->macBarcodeLabel->setText(macAddress);
#endif
}

#ifdef WITH_POPPLER_QT5
void MainWindow::printMAC()
{
    if(!m_esp->isPortOpen()){
        return;
    }

    QString macAddress =  m_esp->macAddress().toUpper();
    QString filename(macAddress +".pdf");
    if(!QFileInfo(filename).exists()){
        return;
    }

    QPrinter printer;
    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() == QDialog::Accepted) {
        Poppler::Document* document = Poppler::Document::load(filename);
        if (document && !document->isLocked()) {
            Poppler::Page* pdfPage = document->page(0);
            if (pdfPage) {
                QImage image = pdfPage->renderToImage(300.0, 300.0);
                if (!image.isNull()) {
                    QPainter painter(&printer);
                    for(int i = 0; i < 3; i++)
                        for(int j = 0; j < 11; j++){
                            painter.drawImage(QPoint(255 * i, 100 * j),image.scaled(250, 90));
                            painter.drawRect(255 * i, 100 * j, 250, 90);
                        }

                    painter.end();
                }
                delete pdfPage;
            }
            delete document;
        }
    }
}
#endif

void MainWindow::copyMAC()
{
    if(m_esp->isPortOpen()){
        QApplication::clipboard()->setText(m_esp->macAddress());
    }
}

void MainWindow::writeFlash()
{  
    if(!m_esp->isPortOpen()){
        return;
    }

    ui->tabWidget->setCurrentIndex(1);

    quint8 flashMode = (quint8)ui->spiMode->currentData().toInt();
    quint8 flashSizeFreq = (quint8)ui->flashSize->currentData().toInt() + (quint8)ui->spiSpeed->currentData().toInt();
    QByteArray flashInfo;
    flashInfo.append(flashMode);
    flashInfo.append(flashSizeFreq);

    int totalWritten = 0;
    for(int i = 0; i < m_filesFields.size(); i++)
    {
        if(!m_filesFields.at(i)->isValid()){
            continue;
        }

        QString filename = m_filesFields.at(i)->filename();
        quint32 address = m_filesFields.at(i)->offset();
        QFile file(filename);
        if(!file.open(QIODevice::ReadOnly)){
            continue;
        }

        m_filesFields.at(i)->setProgress(0);
        QByteArray image = file.readAll();
        quint32 blocks = ESPFlasher::Tools::divRoundup(image.size(), ESP_FLASH_BLOCK);
        if(!m_esp->flashBegin(blocks * ESP_FLASH_BLOCK, address)){
            ui->logList->addEntry("Failed to enter Flash download mode", LogList::Error);
            return;
        }

        quint32 seq = 0;
        int written = 0, pos = 0;
        while(pos < image.size())
        {
            ui->logList->addEntry(QString::asprintf(WRITE_FLASH_PROGRESS,
                                                    QFileInfo(filename).fileName().toLatin1().data(),
                                                    address + seq * ESP_FLASH_BLOCK,
                                                    100 * (seq + 1) / blocks), LogList::Info, seq);
            m_filesFields.at(i)->setProgress(100 * (seq + 1) / blocks);

            QByteArray block = image.mid(pos, ESP_FLASH_BLOCK);
            if (address == 0 && seq == 0 && block.at(0) == '\xe9'){
                block = block.mid(0, 2) + flashInfo + block.mid(4);
            }
            int blockSize = block.size();
            for(int j = 0; j < (ESP_FLASH_BLOCK - blockSize); j++){
                block.append("\xff", 1);
            }
            if(!m_esp->flashBlock(block, seq)){
                ui->logList->addEntry(QString("Failed to write to target Flash after seq %1").arg(seq), LogList::Error);
                return;
            }

            seq += 1;
            pos += ESP_FLASH_BLOCK;
            written += block.size();

        }

        totalWritten += written;
        file.close();
        ui->logList->addEntry(QString::asprintf("Wrote %d bytes at 0x%08X",  written, address), LogList::Info, seq);
    }

    if(flashMode == DIO){
        m_esp->flashUnlockDIO();
    }

    QMessageBox::information(this, "", QString("Flash complete! (Wrote %1 bytes).").arg(totalWritten), QMessageBox::Ok);
}

void MainWindow::readFlash()
{
    if(!m_esp->isPortOpen()){
        return;
    }

    if(ESPFlasher::Tools::openDialog (m_inputDialog, FlashInputDialog::FileField | FlashInputDialog::AddressField | FlashInputDialog::SizeField, this) == QDialog::Accepted)
    {
        QString fileName = m_inputDialog->filename();
        quint32 address = m_inputDialog->address();
        quint32 size = m_inputDialog->size();

        QFile file(fileName);
        if(file.open(QIODevice::WriteOnly)){
            ui->logList->addEntry(QString::asprintf("Reading %d bytes at 0x%08X...",  size, address));
            QByteArray data = m_esp->flashRead(address, 1024, ESPFlasher::Tools::divRoundup(size, 1024)).mid(0, size);
            file.write(data);
            ui->logList->addEntry(QString::asprintf("Reading %d bytes at 0x%08X...done",  data.size(), address), LogList::Info, 1);
            file.close();
        }
    }

    delete m_inputDialog;
}

void MainWindow::eraseFlash()
{
    if(m_esp->isPortOpen() && m_esp->flashErase())
    {
        ui->logList->addEntry("Flash content deleted.", LogList::Warning);
    }
}

void MainWindow::loadRam()
{
    if(!m_esp->isPortOpen()){
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this, QLatin1String(LOAD_IMAGE_TITLE), QDir::currentPath(), QLatin1String(BIN_FILES_FILTER));
    if(fileName.isEmpty()){
        return;
    }

    ESPFlasher::ESPFirmwareImage image(fileName);

    ui->logList->addEntry(tr("RAM boot..."));
    for(int i = 0; i < image.segments().size(); i++){
        ESPFlasher::Segment segment = image.segments().at(i);
        ui->logList->addEntry(QString::asprintf("Downloading %d bytes at %08X...", segment.size, segment.offset), LogList::Info, i);
        m_esp->memBegin(segment.size, ESPFlasher::Tools::divRoundup(segment.size, ESP_RAM_BLOCK), ESP_RAM_BLOCK, segment.offset);
        int seq = 0, pos = 0;
        while(pos < segment.data.size()){
            m_esp->memBlock(segment.data.mid(pos, ESP_RAM_BLOCK), seq);
            pos += ESP_RAM_BLOCK;
            seq++;
        }
    }
    ui->logList->addEntry(QString::asprintf("All segments done, executing at %08X", image.entryPoint()));
    m_esp->memFinish(image.entryPoint());
}

void MainWindow::dumpMemory()
{

    if(!m_esp->isPortOpen()){
        return;
    }

    if(ESPFlasher::Tools::openDialog (m_inputDialog, FlashInputDialog::FileField | FlashInputDialog::AddressField | FlashInputDialog::SizeField, this) == QDialog::Accepted)
    {
        QString fileName = m_inputDialog->filename();
        quint32 address = m_inputDialog->address();
        quint32 size = m_inputDialog->size();

        if(fileName.isEmpty()){
            return;
        }

        int k = 0;
        QFile file(fileName);
        if(file.open(QIODevice::WriteOnly)){
            for(quint32 i = 0; i < size / 4; i++){
                char bytes[4];
                quint32 value = m_esp->readReg(address + (i * 4));
                ESPFlasher::quint32toBytes(value, &bytes[0]);
                file.write(bytes, 4);
                if(file.pos() % 1024 == 0){
                    ui->logList->addEntry(QString::asprintf("%lld bytes read... (%lld %%)", file.pos(), file.pos() * 100 / size), LogList::Info, k++);
                }
            }
            file.close();
        }
    }

    delete m_inputDialog;
}

void MainWindow::readMemory()
{
    if(!m_esp->isPortOpen()){
        return;
    }

    if(ESPFlasher::Tools::openDialog (m_inputDialog, FlashInputDialog::AddressField, this) == QDialog::Accepted)
    {
        quint32 address = m_inputDialog->address();
        quint32 value = m_esp->readReg(address);
        ui->logList->addEntry(QString::asprintf("0x%08X = 0x%08X", address, value));
    }

    delete m_inputDialog;
}

void MainWindow::writeMemory()
{
    if(!m_esp->isPortOpen()){
        return;
    }

    if(ESPFlasher::Tools::openDialog (m_inputDialog, FlashInputDialog::AddressField | FlashInputDialog::ValueField | FlashInputDialog::MaskField, this) == QDialog::Accepted)
    {
        quint32 address = m_inputDialog->address();
        quint32 value = m_inputDialog->value();
        quint32 mask = m_inputDialog->mask();

        m_esp->writeReg(address, value, mask, 0);
        ui->logList->addEntry(QString::asprintf("Wrote 0x%08X, mask 0x%08X to 0x%08X", value, mask, address));
    }

    delete m_inputDialog;
}

void MainWindow::makeImage()
{
    quint8 flashMode = (quint8)ui->spiMode->currentData().toInt();
    quint8 flashSizeFreq = (quint8)ui->flashSize->currentData().toInt() + (quint8)ui->spiSpeed->currentData().toInt();
    ESPFlasher::Tools::openDialog (m_makeImageDialog, flashMode, flashSizeFreq, this);
}

void MainWindow::runImage()
{
    if(!m_esp->isPortOpen()){
        return;
    }

    m_esp->run();
    ui->logList->addEntry("Image is running on device...");
}

void MainWindow::importImageList()
{
    QSettings settings;

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import from text file"),
                                                    settings.value("workingDir", QDir::currentPath()).toString(), tr("Text Files (*.txt)"));
    if(fileName.isEmpty()){
        return;
    }

    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        int i = 0;
        while(!file.atEnd()){
            QString line = QString::fromLocal8Bit(file.readLine().simplified().data());
            if(!line.startsWith("#")){
                QList<QString> fileAddress = line.split(":");
                if(fileAddress.size() == 2){
                    m_filesFields.at(i)->setFilename(fileAddress.at(0));
                    m_filesFields.at(i)->setOffset(fileAddress.at(1).toInt(0, 16));
                    i++;
                }
            }
        }
        file.close();
        ui->tabWidget->setCurrentIndex(1);
    }

    settings.setValue("workingDir", QFileInfo(fileName).absolutePath());
}

void MainWindow::exportImageList()
{
    QSettings settings;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to text file"),
                                                    settings.value("workingDir", QDir::currentPath()).toString(), tr("Text Files (*.txt)"));
    if(fileName.isEmpty()){
        return;
    }

    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);
        for(int i = 0; i < m_filesFields.size(); i++){
            if(m_filesFields.at(i)->isValid()){
                out << m_filesFields.at(i)->filename() << ":"
                    << QString::asprintf("0x%05x", m_filesFields.at(i)->offset()) << "\n";
            }
        }
        file.close();
    }

    settings.setValue("workingDir", QFileInfo(fileName).absolutePath());
}

void MainWindow::espCmdStarted()
{
    setCursor(Qt::WaitCursor);

    ui->openBtn->setEnabled(false);
    ui->writeFlashBtn->setEnabled(false);
    ui->readFlashBtn->setEnabled(false);
    ui->readMemoryBtn->setEnabled(false);
    ui->writeMemoryBtn->setEnabled(false);
    ui->dumpMemoryBtn->setEnabled(false);
    ui->runImageBtn->setEnabled(false);
    ui->loadRamBtn->setEnabled(false);
    ui->eraseFlashBtn->setEnabled(false);
}

void MainWindow::espCmdFinished()
{
    setCursor(Qt::ArrowCursor);

    ui->openBtn->setEnabled(true);
    ui->writeFlashBtn->setEnabled(true);
    ui->readFlashBtn->setEnabled(true);
    ui->readMemoryBtn->setEnabled(true);
    ui->writeMemoryBtn->setEnabled(true);
    ui->dumpMemoryBtn->setEnabled(true);
    ui->runImageBtn->setEnabled(true);
    ui->loadRamBtn->setEnabled(true);
    ui->eraseFlashBtn->setEnabled(true);
}

void MainWindow::espError(const QString &errorText)
{
    ui->logList->addEntry(QString("A fatal error occurred: %1").arg(errorText), LogList::Error);
    espCmdFinished();
}




