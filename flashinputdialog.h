#ifndef FLASHINPUTDIALOG_H
#define FLASHINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class FlashInputDialog;
}

class FlashInputDialog : public QDialog
{
    Q_OBJECT

public:

    enum FieldsFlag {
        FileField = 0x01,
        AddressField = 0x02,
        ValueField = 0x04,
        SizeField = 0x08,
        MaskField = 0x16
    };
    Q_DECLARE_FLAGS(Fields, FieldsFlag)

    explicit FlashInputDialog(Fields fields, QWidget *parent = 0);
    ~FlashInputDialog();

    QString filename();
    int address();
    int value();
    int size();
    int mask();

private slots:
    void setFile();

private:
    Ui::FlashInputDialog *ui;
    Fields m_fields;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FlashInputDialog::Fields)

#endif // FLASHINPUTDIALOG_H
