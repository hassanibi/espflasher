#include "hexlineedit.h"

HexLineEdit::HexLineEdit (QWidget * parent):
    QLineEdit (parent)
{
    setInputMask("\\0\\xhhhhhhhh");
    setClearButtonEnabled(true);
    setCursorPosition(2);
}

