#include "chatserveradmin.h"
#include "ui_chatserveradmin.h"
#include <QApplication>

ChatServerAdmin::ChatServerAdmin(QString cname, quint16 port, QWidget *parent):
    QWidget(parent),
    ui(new Ui::ChatServerAdmin)
{
    ui->setupUi(this);
    ui->clientnamelineEdit->setText(cname);                                 // 생성자의 이니셜라이저로 받아온 cname값 설정
    ui->portlineEdit->setText(QString::number(port));                       // 생성자의 이니셜라이저로 받아온 port값 설정
    ui->clientnamelineEdit->setReadOnly(true);                              // clientnamelineEdit을 읽기전용으로 설정
    ui->portlineEdit->setReadOnly(true);                                    // portlineEdit을 읽기전용으로 설정
    ui->admintextEdit->setReadOnly(true);                                   // admintextEdit을 읽기전용으로 설정
    connect(ui->quitpushButton, SIGNAL(clicked()), SLOT(close()));          // quitpushbutton 클릭시 close슬롯 실행
}

ChatServerAdmin::~ChatServerAdmin()                                         // ChatServerAdmin 소멸자
{
    delete ui;
}

void ChatServerAdmin::on_sendpushButton_clicked()                           // sendbutton 클릭시 발생되는 슬롯
{
    QString name = ui->clientnamelineEdit->text();                          // clientnamelineEdit의 텍스트를 name에 저장
    QString str = ui->adminlineEdit->text();                                // adminlineEdit의 텍스트를 str에 저장
    if(str.length()){                                                       // str의 데이터가 있다면
        QByteArray bytearray;                                               // bytearray변수선언
        bytearray = str.toUtf8( );                                          // str을 utf8로 인코딩하여 저장
        ui->admintextEdit->append("<font color=red>나</font> : " + str);    // 화면에 표시 : 앞에 '나'라고 추가
    }
    emit sendData(name,str);                                                // server에 name과 str을 보내기위한 슬롯
}

void ChatServerAdmin::clientChatRecv(QString name, QString str)             // 서버에서 받은 name과 str을 받기위한 슬롯
{
    if(str.length())                                                        // str의 데이터가 있다면
    {
        ui->admintextEdit->append(name + " : " + str);                      // admintextEdit에 name과 str 추가
    }
}
