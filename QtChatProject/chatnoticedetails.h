#ifndef CHATNOTICEDETAILS_H
#define CHATNOTICEDETAILS_H

#include "qtreewidget.h"
#include <QWidget>

namespace Ui {
class ChatNoticeDetails;
}

class ChatNoticeDetails : public QWidget
{
    Q_OBJECT

public:
    explicit ChatNoticeDetails(QWidget *parent = nullptr);  // ChatNoticeDetails 생성자
    ~ChatNoticeDetails();                                   // ChatNoticeDetails 소멸자

    void setClientList(QHash<quint16, QString>);            // server에서 현재 chatting에 접속한 List를 받기위한 슬롯

protected:
    void closeEvent(QCloseEvent*) override;                 // Notice창이 닫힐시 실행되는 이벤트

signals:
    void closeNotice();                                     // ServerNotice버튼활성화를 위한 시그널
    void sendNoticeData(QString);                           // server에 str 내용 보내기위한 signal

private slots:
    void on_sendpushButton_clicked();                       // send 버튼 클릭시 발동되는 슬롯

private:
    Ui::ChatNoticeDetails *ui;                              // Ui를 사용하기위한 멤버변수

    QList<QTreeWidgetItem*> chatList;                       // stringList 추가하기위한 QList객체
};

#endif // CHATNOTICEDETAILS_H
