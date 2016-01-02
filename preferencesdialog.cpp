#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QSettings>
#include <QFileDialog>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);

    connect(this, SIGNAL(accepted()), this, SLOT(saveSettings()));
    connect(ui->useSystemPATH, SIGNAL(clicked(bool)), ui->tcPathLineEdit, SLOT(setDisabled(bool)));
    connect(ui->useSystemPATH, SIGNAL(clicked(bool)), ui->tcPathBtn, SLOT(setDisabled(bool)));
    connect(ui->tcPathBtn, SIGNAL(clicked(bool)), this, SLOT(setToolchainPath()));

    loadSettings();
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::setToolchainPath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Xtensa toolchain path"), QDir::currentPath(), QFileDialog::ShowDirsOnly
                                                         | QFileDialog::DontResolveSymlinks);

    if(!dir.isEmpty()){
        ui->tcPathLineEdit->setText(dir);
    }
}

void PreferencesDialog::loadSettings()
{
    QSettings settings;

    ui->useSystemPATH->setChecked(settings.value("useSystemPATH", true).toBool());
    ui->tcPathLineEdit->setText(settings.value("tcPath", "").toString());
    ui->tcPathLineEdit->setEnabled(!ui->useSystemPATH->isChecked());
    ui->tcPathBtn->setEnabled(!ui->useSystemPATH->isChecked());
    ui->useDarkTheme->setChecked(settings.value("useDarkTheme", true).toBool());
}

void PreferencesDialog::saveSettings()
{
    QSettings settings;

    settings.setValue("useSystemPATH", ui->useSystemPATH->isChecked());
    settings.setValue("tcPath", ui->tcPathLineEdit->text());
    settings.setValue("useDarkTheme", ui->useDarkTheme->isChecked());

    //accept();
}
