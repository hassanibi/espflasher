#include "imagechooser.h"
#include "ui_imagechooser.h"
#include "imageinfodialog.h"
#include "tools.h"

#include <QFileDialog>
#include <QSettings>
#include <QFileInfo>

ImageChooser::ImageChooser(bool enabled, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageChooser),
    m_infoDialog()
{
    ui->setupUi(this);

    ui->lineEditFile->setEnabled(enabled);
    ui->lineEditOffset->setEnabled(enabled);
    ui->checkBox->setChecked(enabled);
    ui->infoBtn->setEnabled(false);

    connect(ui->toolButton, SIGNAL(clicked(bool)), this, SLOT(setFile()));
    connect(ui->checkBox, SIGNAL(clicked(bool)), ui->lineEditFile, SLOT(setEnabled(bool)));
    connect(ui->checkBox, SIGNAL(clicked(bool)), ui->lineEditOffset, SLOT(setEnabled(bool)));
    connect(ui->infoBtn, SIGNAL(clicked(bool)), this, SLOT(showInfo()));
    connect(ui->lineEditFile, SIGNAL(textChanged(QString)), this, SLOT(filenameChnaged(QString)));
}

ImageChooser::~ImageChooser()
{
    delete ui;
}

void ImageChooser::filenameChnaged(const QString &filename)
{
    QFileInfo file(filename);
    ui->infoBtn->setEnabled(file.isFile() && file .exists());
    setProgress(0);
}

void ImageChooser::setFile()
{
    QSettings settings;
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"),
                                                    settings.value("workingDir", QDir::currentPath()).toString(),
                                                    tr("Binary Files (*.bin)"));

    if(!fileName.isEmpty()){
        ui->checkBox->setChecked(true);
        ui->lineEditOffset->setEnabled(true);
        ui->lineEditFile->setEnabled(true);
        ui->lineEditFile->setText(fileName);

        settings.setValue("workingDir", QFileInfo(fileName).absolutePath());
    }
}


void ImageChooser::showInfo()
{
    if(!filename().isEmpty()){
      ESPFlasher::Tools::openDialog (m_infoDialog, filename(), this);
    }
}


QString ImageChooser::filename()
{
    return ui->lineEditFile->text();
}

void ImageChooser::setFilename(const QString &fn)
{
    ui->lineEditFile->setText(fn);
    ui->lineEditFile->setEnabled(true);
    ui->checkBox->setChecked(true);
}

quint32 ImageChooser::offset()
{
    QString of = ui->lineEditOffset->text();
    return !of.isEmpty() ? of.toUInt(0, 16) : 0;
}

void ImageChooser::setOffset(quint32 of)
{
    ui->lineEditOffset->setText(QString::asprintf("0x%05x", of));
    ui->lineEditOffset->setEnabled(true);
}

bool ImageChooser::isValid()
{
    return ui->checkBox->isChecked() && !filename().isEmpty() ;//&& offset() > -1;
}

void ImageChooser::setProgress(int progress)
{
    ui->lineEditFile->setProgress(progress);
}
