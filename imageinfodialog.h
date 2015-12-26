#ifndef IMAGEINFODIALOG_H
#define IMAGEINFODIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class ImageInfoDialog;
}

class ImageInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImageInfoDialog(const QString &filename, QWidget *parent = 0);
    ~ImageInfoDialog();

private:
    Ui::ImageInfoDialog *ui;
};

#endif // IMAGEINFODIALOG_H
