#ifndef LOGTHREAD_H
#define LOGTHREAD_H

#include <QThread>
#include <QList>

class QTreeWidgetItem;                              // QTreeWidgetItem클래스 참조

class LogThread : public QThread
{
    Q_OBJECT
public:
    explicit LogThread(QObject *parent = nullptr);  // LogThread생성자

private:
    void run();                                     // LogThread를 실행하기위한 멤버함수

    QList<QTreeWidgetItem*> itemList;               // OrderTreeWidget의 항목을 저장하기위한 멤벼변수
    QString filename;

signals:
    void send(int data);

public slots:
    void appendData(QTreeWidgetItem*);              // server의 로그기록을 저장하기위한 슬롯
    void saveData();                                // 현재 itemList에 저장된값을 파일로그로 저장하기위한 함수
};

#endif // LOGTHREAD_H
