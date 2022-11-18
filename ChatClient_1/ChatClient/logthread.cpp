#include "logthread.h"

#include <QTreeWidgetItem>
#include <QFile>
#include <QDateTime>

LogThread::LogThread(QObject *parent)
    : QThread{parent}
{
    QString format = "yyyyMMdd";
    QString date = QDateTime::currentDateTime().toString(format);
    filename = QString("clientlog_%1.txt").arg(date);
}

void LogThread::run()
{
    Q_FOREVER {
        saveData();
        sleep(60);      // 1분마다 저장
    }
}

void LogThread::appendData(QString logitem)
{
    itemList.append(logitem);
}

void LogThread::saveData()
{
    if(itemList.count() > 0) {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
            return;

        QTextStream out(&file);
        foreach(auto item, itemList) {
            out << item << "\n";
        }
        itemList.clear();
        file.close();
    }
}
