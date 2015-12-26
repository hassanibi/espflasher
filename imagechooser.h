#ifndef IMAGECHOOSER_H
#define IMAGECHOOSER_H

#include <QWidget>
#include <QPointer>

namespace Ui {
class ImageChooser;
}

class ImageInfoDialog;

class ImageChooser : public QWidget
{
    Q_OBJECT

public:
    explicit ImageChooser(bool enabled = false, QWidget *parent = 0);
    ~ImageChooser();


    QString filename();
    void setFilename(const QString &fn);
    int offset();
    void setOffset(int of);
    bool isValid();

    void setProgress(int progress);

private slots:
    void setFile();
    void showInfo();
    void filenameChnaged(const QString &filename);

private:
    Ui::ImageChooser *ui;
    QPointer<ImageInfoDialog> m_infoDialog;
};

#endif // IMAGECHOOSER_H
