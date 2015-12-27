#include "loglist.h"

#include <QDateTime>
#include <QList>

LogList::LogList(QWidget *parent) : QListWidget(parent)
{

}

void LogList::addEntry(const QString &str, LogLevel level, int currentRow)
{
    QListWidgetItem *item = new QListWidgetItem(this);

    QFont font = item->font();
    switch (level) {
    case Info:
        item->setForeground(Qt::white); break;
    case Warning:
        item->setForeground(Qt::yellow); break;
    case Error:
        item->setForeground(Qt::red); break;
    default:
        break;
    }
    font.setBold(true);
    item->setFont(font);
    item->setText(QString("%1 %3").arg(QDateTime::currentDateTime().toString("hh:mm:ss")).arg(str));

    if(currentRow > 0)
        insertItem(count() - 1, item);
    else
        addItem(item);

    scrollToBottom();
}

