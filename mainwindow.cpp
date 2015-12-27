#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "esprom.h"
#include "espfirmwareimage.h"
#include "tools.h"
#include "imagechooser.h"
#include "flashinputdialog.h"
#include "loglist.h"
#include "barcodeprinter.h"
#include "makeimagedialog.h"
#include "versiondialog.h"

#include <poppler/qt5/poppler-qt5.h>

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

    fillComboBoxes();

    connect(m_esp, SIGNAL(commandStarted(ESPCommand)), this, SLOT(espCmdStarted()));
    connect(m_esp, SIGNAL(commandFinished(ESPCommand)), this, SLOT(espCmdFinished()));

    connect(ui->actionImport_image_file_list, SIGNAL(triggered(bool)), this, SLOT(importImageList()));
    connect(ui->actionExport_image_file_list, SIGNAL(triggered(bool)), this, SLOT(exportImageList()));
    connect(ui->openBtn, SIGNAL(clicked(bool)), this, SLOT(open()));
    connect(ui->writeFlashBtn, SIGNAL(clicked(bool)), SLOT(writeFlash()));
    connect(ui->readFlashBtn, SIGNAL(clicked(bool)), SLOT(readFlash()));
    connect(ui->loadRamBtn, SIGNAL(clicked(bool)), SLOT(loadRam()));
    connect(ui->dumpMemoryBtn, SIGNAL(clicked(bool)), SLOT(dumpMemory()));
    connect(ui->readMemoryBtn, SIGNAL(clicked(bool)), SLOT(readMemory()));
    connect(ui->writeMemoryBtn, SIGNAL(clicked(bool)), SLOT(writeMemory()));
    connect(ui->makeImageBtn, SIGNAL(clicked(bool)), this, SLOT(makeImage()));

    connect(ui->printMacBtn, SIGNAL(clicked(bool)), SLOT(printMAC()));
    connect(ui->copyMacBtn, SIGNAL(clicked(bool)), SLOT(copyMAC()));

    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(openAbout()));

    for(int i = 0; i < 4; i++){
        addFileField();
    }

    enableActions();

    ui->logList->addEntry(QLatin1String("Ready."));
}

MainWindow::~MainWindow()
{
    delete m_esp;
    delete ui;
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

void MainWindow::fillComboBoxes()
{
    QList<QSerialPortInfo>	availablePorts = QSerialPortInfo::availablePorts();
    for(int i = 0; i < availablePorts.size(); i++){
        ui->serialPort->addItem(availablePorts.at(i).portName(), availablePorts.at(i).systemLocation());
    }

    static QList<qint32> standardBaudRates = QSerialPortInfo::standardBaudRates();
    for(int i = 0; i < standardBaudRates.size(); i++){
        if(standardBaudRates.at(i) > 9599 && standardBaudRates.at(i) < 115201)
            ui->baudRate->addItem(QString::number(standardBaudRates.at(i)), standardBaudRates.at(i));
    }
    ui->baudRate->setCurrentText(QString::number(QSerialPort::Baud115200));

    QList<QString> modes;
    QList<int> modesData;
    modes << "QIO" << "QOUT" << "DIO" << "DOUT";
    modesData << 0 << 1 << 2 << 3;
    for(int i = 0; i < modes.size(); i++){
        ui->spiMode->addItem(modes.at(i), modesData.at(i));
    }
    ui->spiMode->setCurrentIndex(0);

    QList<int> sizes, sizesData;
    sizes << 2 << 4 << 8 << 16 << 32;
    sizesData << 0x10 << 0x00 << 0x20 << 0x30 << 0x40;
    for(int i = 0; i < sizes.size(); i++){
        ui->flashSize->addItem(QString("%1 MBit").arg(sizes.at(i)), sizesData.at(i));
    }
    ui->flashSize->setCurrentIndex(1);

    QList<int> speeds, speedsData;
    speeds << 20 << 26 << 40 << 80;
    speedsData << 2 << 1 << 0 << 0xf;
    for(int i = 0; i < speeds.size(); i++){
        ui->spiSpeed->addItem(QString("%1 Mhz").arg(speeds.at(i)), speedsData.at(i));
    }
    ui->spiSpeed->setCurrentIndex(2);
}

void MainWindow::enableActions()
{
    bool deviceConnected = m_esp->isOpen();
    ui->writeFlashBtn->setEnabled(deviceConnected);
    ui->readFlashBtn->setEnabled(deviceConnected);
    ui->readMemoryBtn->setEnabled(deviceConnected);
    ui->writeMemoryBtn->setEnabled(deviceConnected);
    ui->dumpMemoryBtn->setEnabled(deviceConnected);
    ui->runImageBtn->setEnabled(deviceConnected);
    ui->loadRamBtn->setEnabled(deviceConnected);
    ui->eraseFlashBtn->setEnabled(deviceConnected);
    ui->macAddressGroup->setEnabled(deviceConnected);
}

void MainWindow::open()
{
    if(m_esp->isOpen()){
        m_esp->close();
        ui->openBtn->setText(tr("Connect"));
        enableActions();
        return;
    }

    m_esp->setSerialPort(ui->serialPort->currentData().toString(), (QSerialPort::BaudRate)ui->baudRate->currentData().toInt());

    ui->openBtn->setText(tr("Connecting..."));
    ui->openBtn->setEnabled(false);
    setCursor(Qt::WaitCursor);

    int tries = 10;
    while (tries-- > 0) {
        if(m_esp->open()){
            ui->logList->addEntry(QString("Connected to %1.").arg(m_esp->portName()));
            break;
        }
    }

    displayMAC();

    ui->openBtn->setText(m_esp->isOpen() ? tr("Disconnect") : tr("Connect"));
    ui->openBtn->setEnabled(true);
    enableActions();
    setCursor(Qt::ArrowCursor);
}

void MainWindow::displayMAC()
{
    if(!m_esp->isOpen()){
        return;
    }

    QString macAddress =  m_esp->macAddress().toUpper();
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
            QImage image = pdfPage->renderToImage(100.0, 100.0);
            if (!image.isNull()) {
                ui->macBarcodeLabel->setPixmap(QPixmap::fromImage(image));
            }
            delete pdfPage;
        }
        delete document;
    }
}

void MainWindow::printMAC()
{
    if(!m_esp->isOpen()){
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
                QImage image = pdfPage->renderToImage(100.0, 100.0);
                if (!image.isNull()) {
                    QPainter painter(&printer);
                    for(int i = 0; i < 3; i++)
                        for(int j = 0; j < 11; j++){
                            painter.drawImage(QPoint(255 * i, 100 * j),image);
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

void MainWindow::copyMAC()
{
    if(m_esp->isOpen()){
        QApplication::clipboard()->setText(m_esp->macAddress());
    }
}

void MainWindow::writeFlash()
{  

    if(!m_esp->isOpen()){
        return;
    }

    quint8 flashMode = (quint8)ui->spiMode->currentData().toInt();
    quint8 flashSizeFreq = (quint8)ui->flashSize->currentData().toInt() + (quint8)ui->spiSpeed->currentData().toInt();
    QByteArray flashInfo;
    flashInfo.append(flashMode);
    flashInfo.append(flashSizeFreq);

    for(int i = 0; i < m_filesFields.size(); i++){
        if(m_filesFields.at(i)->isValid()){

            QString filename = m_filesFields.at(i)->filename();
            quint32 address = m_filesFields.at(i)->offset();
            QFile file(filename);

            m_filesFields.at(i)->setProgress(0);

            if(file.open(QIODevice::ReadOnly))
            {
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

                    ui->logList->addEntry(QString::asprintf("Writing '%s' at 0x%08x... (%d %%)",
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

                file.close();

                ui->logList->addEntry(QString::asprintf("Wrote %d bytes at 0x%08x",  written, address), LogList::Info, seq);
            }
        }
    }

    if(flashMode == DIO){
        m_esp->flashUnlockDIO();
    }else{
        m_esp->flashBegin(0, 0);
        m_esp->flashFinish(true);
    }
}

void MainWindow::readFlash()
{
    if(!m_esp->isOpen()){
        return;
    }

    if(ESPFlasher::Tools::openDialog (m_inputDialog, FlashInputDialog::FileField | FlashInputDialog::AddressField | FlashInputDialog::SizeField, this) == QDialog::Rejected){
        return;
    }

    QString fileName = m_inputDialog->filename();
    int address = m_inputDialog->address();
    int size = m_inputDialog->size();

    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly)){
        file.write(m_esp->flashRead(address, 1024, ESPFlasher::Tools::divRoundup(size, 1024)).mid(0, size));
        file.close();
    }
}

void MainWindow::loadRam()
{
    if(!m_esp->isOpen()){
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Load ESP Firmware Image"),
                                                    QDir::currentPath(), tr("Binary Files (*.bin)"));
    if(fileName.isEmpty()){
        return;
    }

    ESPFlasher::ESPFirmwareImage image(fileName);

    ui->logList->addEntry(tr("RAM boot..."));
    for(int i = 0; i < image.segments().size(); i++){
        ESPFlasher::Segment segment = image.segments().at(i);
        ui->logList->addEntry(QString::asprintf("Downloading %d bytes at %08x...", segment.size, segment.offset), LogList::Info, i);
        m_esp->memBegin(segment.size, ESPFlasher::Tools::divRoundup(segment.size, ESP_RAM_BLOCK), ESP_RAM_BLOCK, segment.offset);
        int seq = 0, pos = 0;
        while(pos < segment.data.size()){
            m_esp->memBlock(segment.data.mid(pos, ESP_RAM_BLOCK), seq);
            pos += ESP_RAM_BLOCK;
            seq++;
        }
    }
    ui->logList->addEntry(QString::asprintf("All segments done, executing at %08x", image.entryPoint()));
    m_esp->memFinish(image.entryPoint());
}

void MainWindow::dumpMemory()
{

    if(!m_esp->isOpen()){
        return;
    }

    if(ESPFlasher::Tools::openDialog (m_inputDialog, FlashInputDialog::AddressField | FlashInputDialog::SizeField, this) == QDialog::Rejected){
        return;
    }

    int address = m_inputDialog->address();
    int size = m_inputDialog->size();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Dump memory to file"),
                                                    QDir::currentPath(), tr("Binary Files (*.bin)"));
    if(fileName.isEmpty()){
        return;
    }

    int k = 0;
    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly)){
        for(int i = 0; i < size / 4; i++){
            char bytes[4];
            ESPFlasher::quint32toBytes(m_esp->readReg(address + (i * 4)), &bytes[0]);
            file.write(bytes, 4);
            if(file.pos() % 1024 == 0){
                ui->logList->addEntry(QString::asprintf("%lld bytes read... (%lld %%)", file.pos(), file.pos() * 100 / size), LogList::Info, k++);
            }
        }
        file.close();
    }

}

void MainWindow::readMemory()
{
    if(!m_esp->isOpen()){
        return;
    }

    if(ESPFlasher::Tools::openDialog (m_inputDialog, FlashInputDialog::AddressField, this) == QDialog::Rejected){
        return;
    }

    int address = m_inputDialog->address();

    ui->logList->addEntry(QString::asprintf("0x%08x = 0x%08x", address, m_esp->readReg(address)));

}

void MainWindow::writeMemory()
{
    if(!m_esp->isOpen()){
        return;
    }

    if(ESPFlasher::Tools::openDialog (m_inputDialog, FlashInputDialog::AddressField | FlashInputDialog::ValueField | FlashInputDialog::MaskField, this) == QDialog::Rejected){
        return;
    }

    int address = m_inputDialog->address();
    int value = m_inputDialog->value();
    int mask = m_inputDialog->mask();

    m_esp->writeReg(address, value, mask, 0);
    ui->logList->addEntry(QString::asprintf("Wrote %08x, mask %08x to %08x", value, mask, address));

}

void MainWindow::makeImage()
{
    ESPFlasher::Tools::openDialog (m_makeImageDialog, this);
}

void MainWindow::runImage()
{
    if(!m_esp->isOpen()){
        return;
    }

    m_esp->run();
}

void MainWindow::importImageList()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import from text file"),
                                                    QDir::currentPath(), tr("Text Files (*.txt)"));
    if(fileName.isEmpty()){
        return;
    }

    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        int i = 0;
        while(!file.atEnd()){
            QString line = QString::fromLocal8Bit(file.readLine().data());
            if(!line.startsWith("#")){
                QList<QString> fileAddress = line.split(QRegExp("\\s"));
                if(fileAddress.size() > 2){
                    m_filesFields.at(i)->setFilename(fileAddress.at(0));
                    m_filesFields.at(i)->setOffset(fileAddress.at(1).toInt(0, 16));
                    i++;
                }

            }
        }
        file.close();

        ui->tabWidget->setCurrentIndex(1);
    }
}

void MainWindow::exportImageList()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to text file"),
                                                    QDir::currentPath(), tr("Text Files (*.txt)"));
    if(fileName.isEmpty()){
        return;
    }

    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);
        for(int i = 0; i < m_filesFields.size(); i++){
            if(m_filesFields.at(i)->isValid()){
                out << m_filesFields.at(i)->filename() << "\t"
                    << QString::asprintf("0x%05x", m_filesFields.at(i)->offset()) << "\n";
            }
        }
        file.close();
    }
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
