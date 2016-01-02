
#include "versiondialog.h"
#include "tools.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>

VersionDialog::VersionDialog(QWidget *parent)
    : QDialog(parent)
{
    if (ESPFlasher::Tools::isLinuxHost())
        setWindowIcon(QIcon(":/images/res/images/app/128.png"));

    setWindowTitle(tr("About ESP Flasher"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QGridLayout *layout = new QGridLayout(this);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    QString ideRev;
#ifdef IDE_REVISION
     ideRev = tr("<br/>From revision %1<br/>").arg(QString::fromLatin1(Constants::IDE_REVISION_STR).left(10));
#endif
     QString buildDateInfo;
     buildDateInfo = tr("<br/>Built on %1 %2<br/>").arg(QLatin1String(__DATE__), QLatin1String(__TIME__));


     const QString description = tr(
        "<h3>%1</h3>"
        "%2<br/>"
        "%3"
        "%4"
        "<br/>"
        "Copyright %5 %6.<br/>"
        "<br/>"
        "The program is provided AS IS with NO WARRANTY OF ANY KIND.<br/>")
        .arg(ESPFlasher::Tools::versionString(),
             ESPFlasher::Tools::buildCompatibilityString(),
             buildDateInfo,
             ideRev,
             "2015",
             "Hassan Id Ben Idder");

    QLabel *copyRightLabel = new QLabel(description);
    copyRightLabel->setWordWrap(true);
    copyRightLabel->setOpenExternalLinks(true);
    copyRightLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QPushButton *closeButton = buttonBox->button(QDialogButtonBox::Close);

    buttonBox->addButton(closeButton, QDialogButtonBox::ButtonRole(QDialogButtonBox::RejectRole | QDialogButtonBox::AcceptRole));
    connect(buttonBox , SIGNAL(rejected()), this, SLOT(reject()));

    QLabel *logoLabel = new QLabel;
    logoLabel->setPixmap(QPixmap(":/images/res/images/app/128.png"));
    layout->addWidget(logoLabel , 0, 0, 1, 1);
    layout->addWidget(copyRightLabel, 0, 1, 4, 4);
    layout->addWidget(buttonBox, 4, 0, 1, 5);
}

bool VersionDialog::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Escape && !ke->modifiers()) {
            ke->accept();
            return true;
        }
    }
    return QDialog::event(event);
}
