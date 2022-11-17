#ifndef CHATSERVERADMIN_H
#define CHATSERVERADMIN_H

#include <QWidget>
#include "chatserver.h"

#define BLOCK_SIZE  1024

namespace Ui {
class ChatServerAdmin;
}

class ChatServerAdmin : public QWidget
{
    Q_OBJECT

public:
    explicit ChatServerAdmin(QString = "", quint16 = 0,     // ChatServerAdmin생성자
                             QWidget *parent = nullptr);
    ~ChatServerAdmin();                                     // ChatServerAdmin소멸자

public slots:
    void clientChatRecv(QString,QString);                   // 서버에서 받은 name과 str을 받기위한 슬롯

private slots:
    void on_sendpushButton_clicked();                       // sendbutton 클릭시 발생되는 슬롯

signals:
    void sendData(QString,QString);                         // server에 name과 str을 보내기위한 슬롯

private:
    Ui::ChatServerAdmin *ui;                                // Ui를 사용하기위한 멤버변수

    QString cname;                                          // 생성자 이니셜라이져로 받아온 cname저장멤버변수
    quint16 port;                                           // 생성자 이니셜라이져로 받아온 port저장멤버변수
};

#endif // CHATSERVERADMIN_H
