#include "loglist.h"

#include <QDateTime>
#include <QList>

LogList::LogList(QWidget *parent) : QListWidget(parent)
{
    setStyleSheet("background-color:rgb(0, 0, 0)");
}

void LogList::addEntry(const QString &str, LogLevel level, int currentRow)
{
    QListWidgetItem *it;

    if(currentRow > 0)
        it = item(count() - 1);
    else
    {
        it = new QListWidgetItem(this);
        addItem(it);
    }

    QFont font = it->font();
    switch (level) {
    case Info:
        it->setForeground(Qt::white); break;
    case Warning:
        it->setForeground(Qt::yellow); break;
    case Error:
        it->setForeground(Qt::red); break;
    default:
        break;
    }

    font.setBold(true);
    it->setFont(font);
    it->setText(QString("%1 %3").arg(QDateTime::currentDateTime().toString("hh:mm:ss")).arg(str));

    scrollToBottom();
}

