#ifndef MAKEIMAGEDIALOG_H
#define MAKEIMAGEDIALOG_H

#include <QDialog>
#include <QList>

class ImageChooser;

namespace Ui {
class MakeImageDialog;
}

class MakeImageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MakeImageDialog(QWidget *parent = 0);
    ~MakeImageDialog();


private slots:
    void addFileField();
    void setFile();
    void makeImage();
    void enableActions(bool enabled = true);

private:
    Ui::MakeImageDialog *ui;
    QList<ImageChooser *> m_filesFields;
};

#endif // MAKEIMAGEDIALOG_H
