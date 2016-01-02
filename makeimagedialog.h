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
    explicit MakeImageDialog(quint8 flashMode, quint8 flashSizeFreq, QWidget *parent = 0);
    ~MakeImageDialog();


private slots:
    void addFileField();
    void setFile();
    void setELFFile();
    void setImageFile();

    void onElfError(const QString &errorText);

    void elf2Image();
    void makeImage();
    void enableActions(bool enabled = true);

private:
    Ui::MakeImageDialog *ui;
    QList<ImageChooser *> m_filesFields;
    quint8 m_flashMode;
    quint8 m_flashSizeFreq;
};

#endif // MAKEIMAGEDIALOG_H
