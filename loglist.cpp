#include "loglist.h"

#include <QDateTime>
#include <QList>

LogList::LogList(QWidget *parent) : QListWidget(parent)
{

}

void LogList::addEntry(const QString &str, LogLevel level, int currentRow)
{
    QListWidgetItem *item = new QListWidgetItem(this);

    item->setText(QString("[%1] %2").arg(QDateTime::currentDateTime().toString("h:m:s")).arg(str));

    QFont font = item->font();
    switch (level) {
    case Info:
        font.setBold(false);
        item->setForeground(Qt::darkGreen); break;
    case Warning:
        font.setBold(false);
        item->setForeground(Qt::darkYellow); break;
    case Error:
        font.setBold(true);
        item->setForeground(Qt::darkRed); break;
    default:
        break;
    }
    item->setFont(font);

    if(currentRow > 0)
        insertItem(count() - 1, item);
    else
        addItem(item);

    scrollToBottom();
}

