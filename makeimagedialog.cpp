#include "makeimagedialog.h"
#include "ui_makeimagedialog.h"
#include "imagechooser.h"
#include "espfirmwareimage.h"
#include "elffile.h"

#include <QFile>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>

MakeImageDialog::MakeImageDialog(quint8 flashMode, quint8 flashSizeFreq, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MakeImageDialog),
    m_flashMode(flashMode),
    m_flashSizeFreq(flashSizeFreq)
{
    ui->setupUi(this);

    connect(ui->makeImageBtn, SIGNAL(clicked(bool)), this, SLOT(makeImage()));
    connect(ui->elf2ImageBtn, SIGNAL(clicked(bool)), this, SLOT(elf2Image()));
    connect(ui->outputFileBtn, SIGNAL(clicked(bool)), this, SLOT(setFile()));
    connect(ui->elfBtn, SIGNAL(clicked(bool)), this, SLOT(setELFFile()));
    connect(ui->imageBtn, SIGNAL(clicked(bool)), this, SLOT(setImageFile()));

    QSettings settings;
    bool useSysPath = settings.value("useSystemPATH").toBool();
    QString tcPath = settings.value("tcPath").toString();
    QString sysPATH = QString::fromLocal8Bit(qgetenv("PATH").data()).toLower();

    ui->warn1Label->setVisible(useSysPath && !sysPATH.contains("xtensa"));
    ui->warn2Label->setVisible(!useSysPath && tcPath.isEmpty());


    for(int i = 0; i < 4; i++){
        addFileField();
    }
}

MakeImageDialog::~MakeImageDialog()
{
    delete ui;
}

void MakeImageDialog::enableActions(bool enabled)
{
    for(int i = 0; i < m_filesFields.size(); i++){
        m_filesFields.at(i)->setEnabled(enabled);
    }
    ui->entryPointGroup->setEnabled(enabled);
}


void MakeImageDialog::addFileField()
{
    if(m_filesFields.size() < 5)
    {
        ImageChooser *fileField = new ImageChooser(false, ui->filesGroup);
        m_filesFields.append(fileField);
        ui->verticalLayoutFG->addWidget(fileField);
    }
}

void MakeImageDialog::setFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save to Image"), QDir::currentPath(), tr("Binary Files (*.bin)"));

    if(!fileName.isEmpty()){
        ui->ouputFileLineEdit->setText(fileName);
    }
}

void MakeImageDialog::setELFFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Save to Image"), QDir::currentPath(), tr("Binary Files (*.*)"));

    if(!fileName.isEmpty()){
        ui->elfLineEdit->setText(fileName);
    }
}

void MakeImageDialog::setImageFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save to Image"), QDir::currentPath(), tr("Binary Files (*.bin)"));

    if(!fileName.isEmpty()){
        ui->imageLineEdit->setText(fileName);
    }
}

void MakeImageDialog::elf2Image()
{
    ui->logList->clear();

    QString elfFilename = ui->elfLineEdit->text();
    QString imageFileame = ui->imageLineEdit->text();

    if(elfFilename.isEmpty() || imageFileame.isEmpty()){
        return;
    }

    QSettings settings;
    bool useSysPath = settings.value("useSystemPATH").toBool();
    QString tcPath = useSysPath ? "" : settings.value("tcPath").toString();

    enableActions(false);
    ESPFlasher::ESPFirmwareImage image;
    ESPFlasher::ELFFile elfFile(elfFilename, tcPath);
    QList<QString> sections;
    QList<QString> starts;
    sections << ".text" << ".data" << ".rodata";
    starts << "_text_start" << "_data_start" << "_rodata_start";

    connect(&elfFile, SIGNAL(elfError(QString)), this, SLOT(onElfError(QString)));

    image.setEntryPoint(elfFile.getEntryPoint());
    image.setFlashMode(m_flashMode);
    image.setFlashSizeFreq(m_flashSizeFreq);

    for(int i = 0; i < sections.size(); i++)
    {
        quint32 address = elfFile.getSymbolAddr(starts.at(i));
        QByteArray data = elfFile.loadSection(sections.at(i));
        image.addSegment(address, data);
        ui->logList->addEntry(QString("Section %1 (%2 bytes at 0x%3)").arg(sections.at(i)).arg(data.size()).arg(address, 1, 16));
    }

    QFileInfo fileInfo(imageFileame);

    image.save(fileInfo.path() + "/0x00000.bin");

    QByteArray data = elfFile.loadSection(".irom0.text");
    quint32 off = elfFile.getSymbolAddr("_irom0_text_start") - 0x40200000;
    QFile file(fileInfo.path() + QString::asprintf("/0x%05x.bin", off));
    if(file.open(QIODevice::WriteOnly)){
        file.write(data.data(), data.size());
        file.close();
    }

    ui->logList->addEntry(QString("Section .irom0.text (%1 bytes at 0x%2)").arg(data.size()).arg(off + 0x40200000, 1, 16));
}

void MakeImageDialog::makeImage()
{
     ui->logList->clear();

    quint32 entryPoint = ui->entryPointLineEdit->text().toUInt(0, 16);
    QString filename = ui->ouputFileLineEdit->text();

    if(filename.isEmpty()){
        return;
    }

    enableActions(false);
    ESPFlasher::ESPFirmwareImage image;
    for(int i = 0; i < m_filesFields.size(); i++){
        if(m_filesFields.at(i)->isValid()){
            QFile file(m_filesFields.at(i)->filename());
            if(file.open(QIODevice::ReadOnly)){
                image.addSegment(m_filesFields.at(i)->offset(), file.readAll());
                file.close();
            }
            m_filesFields.at(i)->setProgress(100);
        }
    }

    image.setEntryPoint(entryPoint);
    image.save(filename);

    enableActions(true);
}

void MakeImageDialog::onElfError(const QString &errorText)
{
    ui->logList->addEntry(errorText, LogList::Error);
}
