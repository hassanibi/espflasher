#include "flashinputdialog.h"
#include "ui_flashinputdialog.h"

#include <QFileDialog>
#include <QDebug>

FlashInputDialog::FlashInputDialog(Fields fields, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FlashInputDialog),
    m_fields(fields)
{
    ui->setupUi(this);

    ui->fileLineEdit->clear();;
    ui->fileLabel->setVisible(m_fields & FileField);
    ui->fileLineEdit->setVisible(m_fields & FileField);
    ui->fileBtn->setVisible(m_fields & FileField);

    ui->addressLineEdit->clear();;
    ui->addressLabel->setVisible(m_fields & AddressField);
    ui->addressLineEdit->setVisible(m_fields & AddressField);

    ui->valueLineEdit->clear();;
    ui->valueLabel->setVisible(m_fields & ValueField);
    ui->valueLineEdit->setVisible(m_fields & ValueField);

    ui->sizeLineEdit->clear();;
    ui->sizeLabel->setVisible(m_fields & SizeField);
    ui->sizeLineEdit->setVisible(m_fields & SizeField);

    ui->maskLineEdit->clear();;
    ui->maskLabel->setVisible(m_fields & MaskField);
    ui->maskLineEdit->setVisible(m_fields & MaskField);

    connect(ui->fileBtn, SIGNAL(clicked(bool)), this, SLOT(setFile()));
}

void FlashInputDialog::setFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select file"), QDir::currentPath(), tr("Binary Files (*.bin)"));

    if(!fileName.isEmpty()){
        ui->fileLineEdit->setText(fileName);
    }
}

FlashInputDialog::~FlashInputDialog()
{
    delete ui;
}

QString FlashInputDialog::filename()
{
    return ui->fileLineEdit->text();
}

int FlashInputDialog::address()
{
    return ui->addressLineEdit->text().toInt(0, 16);
}

int FlashInputDialog::value()
{
    return ui->valueLineEdit->text().toInt(0, 16);
}

int FlashInputDialog::size()
{
    return ui->sizeLineEdit->text().toInt(0, 16);
}

int FlashInputDialog::mask()
{
    return ui->maskLineEdit->text().toInt(0, 16);
}
