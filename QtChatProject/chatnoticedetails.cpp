#include "chatnoticedetails.h"
#include "ui_chatnoticedetails.h"

ChatNoticeDetails::ChatNoticeDetails(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatNoticeDetails)
{
    ui->setupUi(this);
    QList<int> sizes;                                                           // TreeWidget과 ToolBox간 거리 조절용
    sizes << 300 << 600;                                                        // 사이즈 지정
    ui->Nsplitter->setSizes(sizes);                                             // 사이즈 세팅

    connect(ui->quitpushButton, SIGNAL(clicked()), SLOT(close()));              // quitpushbutton클릭시그널-> close슬롯 실행
}

ChatNoticeDetails::~ChatNoticeDetails()                                         // ChatNoticeDetails 소멸자
{
    delete ui;
}

void ChatNoticeDetails::closeEvent(QCloseEvent*)                                // Notice창이 닫힐시 실행되는 이벤트
{
    emit closeNotice();                                                         // ServerNotice버튼활성화를 위한 시그널
}

void ChatNoticeDetails::setClientList(QHash<quint16, QString> clientNameHash)   // server에서 현재 chatting에 접속한 List를 받기위한 슬롯
{
    QStringList strings;                                                        // String을 담기위한 QStringList변수선언
    QHash<quint16, QString>::iterator i;                                        // iterator변수선언
    for (i = clientNameHash.begin(); i != clientNameHash.end(); ++i) {          // 가져온 해쉬List iterator
        strings << QString::number(i.key()) << i.value();                       // stringList에 key값 및 value값 설정
        chatList.append(new QTreeWidgetItem(strings));                          // chatList에 stringList 추가
        strings.clear();                                                        // stringsList 클리어
    }
    ui->clientTreeWidget->addTopLevelItems(chatList);                           // clientTreeWidget에 chatList아이템 추가
    ui->clientTreeWidget->resizeColumnToContents(0);                            // clientTreeWidget 아이템 간격 조절
}

void ChatNoticeDetails::on_sendpushButton_clicked()                             // send 버튼 클릭시 발동되는 슬롯
{
    QString str = ui->noticelineEdit->text();                                   // noticelineEdit의 text추출
    if(str.length()){                                                           // str의 값이 존재한다면
        QByteArray bytearray;                                                   // bytearray 변수선언
        bytearray = str.toUtf8( );                                              // str을 utf8로 인코딩하여 저장

        ui->noticetextEdit->append("<font color=red>공지사항</font> : " + str);  // 화면에 표시 : 앞에 '공지사항'라고 추가
    }
    emit sendNoticeData(str);                                                   // server에 str 내용 보내기위한 signal
}

