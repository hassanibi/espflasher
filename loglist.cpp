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

    switch (level) {
    case Info:
        item->setForeground(Qt::darkGreen); break;
    case Warning:
        item->setForeground(Qt::darkYellow); break;
    case Error:
        item->setForeground(Qt::darkRed); break;
    default:
        break;
    }

    if(currentRow > 0)
        insertItem(count() - 1, item);
    else
        addItem(item);

    scrollToBottom();
}

