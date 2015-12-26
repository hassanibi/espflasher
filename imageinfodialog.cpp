#include "imageinfodialog.h"
#include "ui_imageinfodialog.h"
#include "espfirmwareimage.h"
#include "tools.h"

#include <QDebug>

ImageInfoDialog::ImageInfoDialog(const QString &filename, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageInfoDialog)
{
    ui->setupUi(this);

    ESPFlasher::ESPFirmwareImage image(filename);

    if(!image.isValid()){
        ui->treeWidget->insertTopLevelItem(0, new QTreeWidgetItem(QStringList() << image.errorText()));
        return;
    }

    ui->treeWidget->insertTopLevelItem(0, new QTreeWidgetItem(QStringList() << "Entry point:" << ((image.entryPoint() != 0) ? QString::asprintf("0x%08x", image.entryPoint()) : "not set")));
    ui->treeWidget->insertTopLevelItem(1, new QTreeWidgetItem(QStringList() << "Segments:"));

    quint8 checksum = ESP_CHECKSUM_MAGIC;
    for(int i = 0; i < image.segments().size(); i++){
        QString text = QString::asprintf("%d: %d bytes at 0x%08x", i+1, image.segments().at(i).size, image.segments().at(i).offset);
        ui->treeWidget->insertTopLevelItem(2+i, new QTreeWidgetItem(QStringList() << "" << text));

        checksum = ESPFlasher::Tools::checksum(image.segments().at(i).data, checksum);
    }

    QString text = QString::asprintf("0x%02x (%s)", image.checksum(), (image.checksum() == checksum) ? "Valid" : "Invalid");
    ui->treeWidget->insertTopLevelItem(2+image.segments().size(), new QTreeWidgetItem(QStringList() << "Checksum:" << text));
}

ImageInfoDialog::~ImageInfoDialog()
{
    delete ui;
}
