#ifndef HEXLINEEDIT_H
#define HEXLINEEDIT_H


#include <QLineEdit>

class QToolButton;

class HexLineEdit: public QLineEdit
{
    Q_OBJECT

  public:
    HexLineEdit (QWidget * parent = 0);

};


#endif // HEXLINEEDIT_H
