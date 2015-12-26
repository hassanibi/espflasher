
#ifndef IMAGELINEEDIT_H
#define IMAGELINEEDIT_H

#include <QLineEdit>

class QToolButton;

class ImageLineEdit: public QLineEdit
{
    Q_OBJECT

  public:
    ImageLineEdit (QWidget * parent = 0);

    void setProgress(int progress);

  protected:
    // QWidget
    virtual void resizeEvent (QResizeEvent * event);
    void paintEvent(QPaintEvent * event);

  private slots:
    void updateClearButtonVisibility ();

  private:
    QToolButton * m_clearBtn;
    int m_progress;
};

#endif // IMAGELINEEDIT_H
