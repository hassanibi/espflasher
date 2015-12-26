#ifndef LOGLIST_H
#define LOGLIST_H

#include <QListWidget>

class LogList : public QListWidget
{
    Q_OBJECT
public:
    explicit LogList(QWidget *parent = 0);

    enum LogLevel {
        Info,
        Warning,
        Error
    };

    void addEntry(const QString &str, LogLevel level = Info, int currentRow = 0);

signals:

public slots:
};

#endif // LOGLIST_H
