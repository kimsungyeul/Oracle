#include "logthread.h"

#include <QTreeWidgetItem>
#include <QFile>
#include <QDateTime>

LogThread::LogThread(QObject *parent)
    : QThread{parent}
{
    QString format = "yyyyMMdd_hhmmss";                                             // date 형식 지정
    filename = QString("serverlog_%1.txt").arg(QDateTime::currentDateTime()         // 파일생성시 뒤에 현재 date 적기위한 QString객체선언
                                               .toString(format));
}

void LogThread::run()                                                               // 스레드 실행
{
    Q_FOREVER {
        saveData();                                                                 // saveData() 멤버함수 실행
        sleep(60);                                                                  // 1분마다 저장
    }
}

void LogThread::appendData(QTreeWidgetItem* item)                                   // item을 받기위한 멤버함수
{
    itemList.append(item);                                                          // 받아온 item을 itemList에 저장
}

void LogThread::saveData()                                                          // 현재 itemList에 저장된값을 파일로그로 저장하기위한 함수
{
    if(itemList.count() > 0) {                                                      // itemList의 item이 하나라도 있다면
        QFile file(filename);                                                       // file객체생성
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) // 쓰기전용,Text형식,추가되는 옵션으로 flag설정
            return;

        QTextStream out(&file);                                                     // 텍스트를 적기위한 TextStream객체생성
        foreach(auto item, itemList) {                                              // itemList iterator
            out << item->text(0) << ", ";                                           // item의 0번째인자인 ip를 저장
            out << item->text(1) << ", ";                                           // item의 1번째인자인 port를 저장
            out << item->text(2) << ", ";                                           // item의 2번째인자인 cid를 저장
            out << item->text(3) << ", ";                                           // item의 3번째인자인 name를 저장
            out << item->text(4) << ", ";                                           // item의 4번째인자인 str를 저장
            out << item->text(5) << "\n";                                           // item의 5번째인자인 date를 저장
        }
        file.close();                                                               // file객체 close
    }
}
