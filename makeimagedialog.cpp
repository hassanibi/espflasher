#include "makeimagedialog.h"
#include "ui_makeimagedialog.h"
#include "imagechooser.h"
#include "espfirmwareimage.h"

#include <QFile>
#include <QFileDialog>
#include <QDebug>

MakeImageDialog::MakeImageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MakeImageDialog)
{
    ui->setupUi(this);

    connect(ui->addFileBtn, SIGNAL(clicked(bool)), this, SLOT(addFileField()));
    connect(ui->makeImageBtn, SIGNAL(clicked(bool)), this, SLOT(makeImage()));
    connect(ui->outputFileBtn, SIGNAL(clicked(bool)), this, SLOT(setFile()));

    for(int i = 0; i < 3; i++){
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
        ImageChooser *fileField = new ImageChooser(true, ui->filesGroup);
        m_filesFields.append(fileField);
        ui->verticalLayoutFG->addWidget(fileField);
    }

    if(m_filesFields.size() == 5)
        ui->addFileBtn->setEnabled(false);
}

void MakeImageDialog::setFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Save to Image"), QDir::currentPath(), tr("Binary Files (*.bin)"));

    if(!fileName.isEmpty()){
        ui->ouputFileLineEdit->setText(fileName);
    }
}

void MakeImageDialog::makeImage()
{
    ESPFlasher::ESPFirmwareImage image;
    quint32 entryPoint = ui->entryPointLineEdit->text().toUInt(0, 16);
    QString filename = ui->ouputFileLineEdit->text();

    if(filename.isEmpty()){
        return;
    }

    enableActions(false);

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
