#include "chatserver.h"
#include "ui_chatserver.h"
#include "logthread.h"
#include "chatserveradmin.h"
#include "chatnoticedetails.h"

#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>
#include <QTcpServer>
#include <QTcpSocket>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <QMenu>
#include <QScrollBar>

#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>

ChatServer::ChatServer(QWidget *parent) :
    QWidget(parent), ui(new Ui::ChatServer), totalSize(0), byteReceived(0)
{
    ui->setupUi(this);

    QList<int> sizes;                                                           // TreeWidget과 ToolBox간 거리 조절용
    sizes << 200 << 600;                                                        // 사이즈 지정
    ui->splitter->setSizes(sizes);                                              // 사이즈 세팅

    /* 채팅서버 생성 */
    chatServer = new QTcpServer(this);                                          // TcpServer객체생성
    connect(chatServer, SIGNAL(newConnection( )), SLOT(clientConnect( )));      // chatServer클래스의 연결시그널이오면 현재클래스의 clientConnect슬롯실행
    if (!chatServer->listen(QHostAddress::Any, PORT_NUMBER)) {                  // 만약 tcpserver에 해당포트번호에대한 신호를 못받는다면
        QMessageBox::critical(this, tr("Chatting Server"),                      // critical 메시지박스 실행
                              tr("Unable to start the server: %1.") \
                              .arg(chatServer->errorString( )));
        close( );                                                               // close qt함수 실행
        return;
    }

    /* 파일서버 생성*/
    fileServer = new QTcpServer(this);                                          // TcpServer객체생성
    connect(fileServer, SIGNAL(newConnection()), SLOT(acceptConnection()));     // fileServer클래스의 연결시그널이오면 현재클래스의 acceptConnection슬롯 실행
    if (!fileServer->listen(QHostAddress::Any, PORT_NUMBER+1)) {                // 만약 fileserver에 해당포트번호+1 에대한 신호를 못받는다면
        QMessageBox::critical(this, tr("Chatting Server"),                      // critical 메시지박스 실행
                              tr("Unable to start the server: %1.") \
                              .arg(fileServer->errorString( )));
        close( );                                                               // close qt함수 실행
        return;
    }

    qDebug("Start listening ...");                                              // 서버생성후 확인용 qDebug

    QAction* inviteAction = new QAction(tr("&Invite"));                         // 초대전용 액션객체 생성
    inviteAction->setObjectName("Invite");                                      // objectname선언
    connect(inviteAction, SIGNAL(triggered()), SLOT(inviteClient()));           // 초대액션 트리거시 초대함수 슬롯 실행

    QAction* removeAction = new QAction(tr("&Kick out"));                       // 강퇴전용 액션객체 생성
    connect(removeAction, SIGNAL(triggered()), SLOT(kickOut()));                // 강퇴액션 트리거시 초대함수 슬롯 실행

    QAction*  privateChatAction = new QAction(tr("&Private Chat"));             // 개인채팅전용 액션객체 생성
    connect(privateChatAction, SIGNAL(triggered()),SLOT(privateChat()));        // 개인채팅액션 트리거시 초대함수 슬롯 실행

    menu = new QMenu;                                                           // 초대액션을 동작하기위한 메뉴생성 멤버변수
    menu->addAction(inviteAction);                                              // 초대액션 추가
    ui->clientTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);          // 위젯의 ContextMenu의 속성을 표시하기 위함

    chatmenu = new QMenu;                                                       // 강퇴 및 개인채팅액션을 동작하기위한 메뉴생성 멤버변수
    chatmenu->addAction(privateChatAction);                                     // 개인채팅 액션 추가
    chatmenu->addAction(removeAction);                                          // 강퇴액션 추가
    ui->chatTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);            // 위젯의 ContextMenu의 속성을 표시하기 위함

    progressDialog = new QProgressDialog(0);                                    // 파일전송시 progressDialog창을 띄우기위한 객체생성
    progressDialog->setAutoClose(true);                                         // dialog 완료시 자동닫힘
    progressDialog->reset();                                                    // dialog설정값 초기화

    logThread = new LogThread(this);                                            // logThread 객체 생성
    logThread->start();                                                         // 스레드 실행

    connect(ui->savePushButton, SIGNAL(clicked()), logThread, SLOT(saveData()));// savebotton클릭시 스레드의 saveData멤버함수 실행

    qDebug() << tr("The server is running on port %1.").arg(chatServer          // 생성자설정값 확인용 qDebug
                                                            ->serverPort());
}

ChatServer::~ChatServer()                                                       // chatServer소멸자
{
    delete ui;

    logThread->terminate();                                                     // 스레드 실행 종료
    chatServer->close( );                                                       // chatServer객체 close
    fileServer->close( );                                                       // fileServer객체 close
}

void ChatServer::clientConnect()                                                // client 연결확인용 슬롯함수
{
    QTcpSocket *clientConnection = chatServer->nextPendingConnection();         // 현재 보류중인 tcp소켓을 연결시 연결된 tcp소켓 객체를 반환
    connect(clientConnection, SIGNAL(readyRead( )), SLOT(receiveData()));       // client연결의 readwrite시그널시 receiveData멤버함수 실행
    connect(clientConnection, SIGNAL(disconnected( )), SLOT(removeClient()));   // client연결이 끊어질시 removeClient멤버함수슬롯 실행
    qDebug("new connection is established...");
}

void ChatServer::receiveData()                                                  // 데이터를 받는 멤버함수슬롯
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));       // sender를 통해 client와의 연결된 tcpsock정보를 받아와 저장
    QByteArray bytearray = clientConnection->read(BLOCK_SIZE);                  // socket에서가져온 데이터블럭의 크기추출

    //chatProtocolType data;
    Chat_Status type;                                                           // 채팅의 목적
    char data[1020];                                                            // 전송되는 메시지/데이터
    memset(data, 0, 1020);                                                      // type의 4바이트를 제외한 1020바이트를 0으로 초기화

    QDataStream in(&bytearray, QIODevice::ReadOnly);                            // bytearray데이터를 열어정보추출을위한 Datastream객체생성
    in.device()->seek(0);                                                       // 데이터스트림 읽는 시작점을 0번째로 세팅
    in >> type;                                                                 // 시작데이터주소부터 4바이트크기의 type데이터 추출
    in.readRawData(data, 1020);                                                 // 나머지 1020바이트 데이터내용을 읽어 data배열에 저장

    QString ip = clientConnection->peerAddress().toString();                    // 연결된 소켓객체의 주소를 뽑아 ip변수에 저장
    quint16 port = clientConnection->peerPort();                                // 연결된 소켓객체의 포트를 뽑아 port변수에 저장
    QString name = QString::fromStdString(data);                                // 데이터스트림으로 뽑은 데이터를 String형으로 변환해 name변수에 저장

    auto finditem = ui->clientTreeWidget->findItems(name,                       // 받아온 이름과 clientTreeWidget에 매칭되는아이템 추출
                                                    Qt::MatchFixedString, 1);
    auto findchatitem = ui->chatTreeWidget->findItems(name,                     // 받아온 이름과 chatTreeWidget에 매칭되는아이템 추출
                                                      Qt::MatchFixedString, 1);
    QTreeWidgetItem* saveitem = new QTreeWidgetItem;                            // 임시저장용 아이템 객체 생성
    qDebug() << ip << " : " << type;                                            // ip와 type 확인용 debug

    switch(type) {                                                              // 프로토콜에 따른 서버동작을 위한 switch문
    case Chat_Login: {                                                          // client보낸 프로토콜이 Chat_Login 이라면
        if(finditem.count() == 0){                                              // 만약 등록된 이름이 아니라면(비회원이라면)
            NonMemaddClient(name);                                              // 임시멤버 id및 item 생성용 멤버함수
        } else {
            foreach(auto item, finditem) {                                      // 등록된 멤버라면 finditem된 name iterator
                if(item->text(0) != "-" && item->text(0) != "-b") {             // item의 0번째인자가 "-" && "-b"가 아니라면
                    item->setText(0, "-");                                      // item의 0번째인자를 "-"로 세팅
                }
            }
        }
        clientList.append(clientConnection);                                    // QList<QTcpSocket*> clientList;
        clientSocketHash[name] = clientConnection;                              // Login단에서 이름을 key값으로 client포트에 바인드된 소켓포트값 및 데이터 저장
        portSocketHash[port] = clientConnection;                                // 현재 서버에 접속된 port에대한 소켓 Data 등록
    }
        break;

    case Chat_In: {                                                             // client보낸 프로토콜이 Chat_In 이라면
        foreach(auto item, finditem) {                                          // 등록된 이름들에서 finditem iterator
            if(item->text(0) != "O" && item->text(0) != "-b") {                 // item의 0번째인자가 "O" && "-b"가 아니라면
                item->setText(0, "O");                                          // 0번째인자를 O로 세팅
            } else {
                item->setText(0, "Ob");                                         // 0번째인자를 Ob로 세팅
            }
            saveitem = item->clone();                                           // 임시저장용 아이템객체에 현재아이템 저장
            clientNameHash[port] = name;                                        // 채팅입장시 Name Hash포트에 포트에따른 이름저장
            ui->clientTreeWidget->takeTopLevelItem(ui->clientTreeWidget         // 선택된 인덱스를 가져와 clientWidget에서 삭제한다
                                                   ->indexOfTopLevelItem(item));
            ui->clientTreeWidget->update();                                     // clientTreeWidget업데이트
        }
        ui->chatTreeWidget->addTopLevelItem(saveitem);                          // 임시저장된 아이템을 chatTreeWidget에 세팅

        ui->chatTreeWidget->resizeColumnToContents(0);                          // 아이템 컬럼 정령
    }
        break;
    case Chat_Talk: {                                                           // client보낸 프로토콜이 Chat_Talk 라면
        foreach(QTcpSocket *sock, clientList) {                                 // clientList에 등록돤 socket들을 iterator
            if(clientNameHash.contains(sock->peerPort()) &&                     // 소켓에서뽑은 포트가 namehash에 등록되잇거나 연결된 tcpipsocket이 iterator중인 socket과 다르다면
                    sock != clientConnection){
                QByteArray sendArray;                                           // datastream으로 보내기위한 데이터프레임인 bytearray선언
                sendArray.clear();                                              // bytearray 클리어
                QDataStream out(&sendArray, QIODevice::WriteOnly);              // bytearray형식의 datastream 선언
                out << Chat_Talk;                                               // 처음 4바이트에 보낼메세지의 프로토콜 선언
                sendArray.append("<font color=lightsteelblue>");                // bytearray에 보낼데이터 append
                sendArray.append(clientNameHash[port].toStdString().data());
                sendArray.append("</font> : ");
                sendArray.append(name.toStdString().data());
                sock->write(sendArray);                                         // socket에 쓰여진 bytearray를 write
            }
        }

        if (clientWindowHash.count() != 0) {                                    // windowhash value의 개수가 하나이상이면
            clientWindowHash[clientNameHash[port]]                              // windowhash에 등록된 이름에대한 이름과 데이터를 clientChat에 보낸다
                    ->clientChatRecv(clientNameHash[port],data);
        }

        QTreeWidgetItem *logitem = new QTreeWidgetItem(ui->messageTreeWidget);  // messageTreeWidget을 부모로하는 widgetitem객체 생성
        logitem->setText(0, ip);                                                // 아이템의 0번째에 고객의 ip 세팅
        logitem->setText(1, QString::number(port));                             // 아이템의 0번째에 고객의 port 세팅
        logitem->setText(2, QString::number(clientIDHash[clientNameHash[port]]));// 아이템의 0번째에 고객의 ID 세팅
        logitem->setText(3, clientNameHash[port]);                              // 아이템의 0번째에 고객의 Name 세팅
        logitem->setText(4, QString(data));                                     // 아이템의 0번째에 고객의 채팅data 세팅
        logitem->setText(5, QDateTime::currentDateTime().toString());           // 아이템의 0번째에 현재시간 세팅
        logitem->setToolTip(4, QString(data));                                  // 아이템툴팁에 데이터 세팅

        for(int i = 0; i < ui->messageTreeWidget->columnCount(); i++) {         // MessageTreeWidget의 컬럼개수만큼 iterator
            ui->messageTreeWidget->resizeColumnToContents(i);                   // column 사이즈 재조정
        }

        ui->messageTreeWidget->addTopLevelItem(logitem);                        // 세팅된 아이템을 messageTreeWidget에 추가

        logThread->appendData(logitem);                                         // logThread등록하기위해 logitem데이터 append
    }
        break;
    case Chat_Out: {                                                            // client보낸 프로토콜이 Chat_Out 이라면
        foreach(auto item, findchatitem) {                                      // chattreewidget에서 검색한 item iterator
            if(item->text(0) != "-" && item->text(0) != "Ob") {                 // item의 0번째 text가 "-" && "Ob"가 아니라면
                item->setText(0, "-");                                          // item의 0번째 text를 "-"로 세팅
            } else {
                item->setText(0, "-b");                                         // item의 0번째 text를 "-b"로 세팅
            }
            saveitem = item->clone();                                           // 임시저장용 아이템객체에 현재아이템 저장
            clientNameHash.remove(port);                                        // clientNameHash에 등록된 현재 port 제거
            ui->chatTreeWidget->takeTopLevelItem(ui->chatTreeWidget             // 현재item을 chatTreeWidget에서 제거
                                                 ->indexOfTopLevelItem(item));
            ui->chatTreeWidget->update();                                       // chatTreeWidget 업데이트
        }
        ui->clientTreeWidget->addTopLevelItem(saveitem);                        // clientTreeWidget에 임시저장했던 item 세팅
        ui->clientTreeWidget->resizeColumnToContents(0);                        // clientTreeWidget의 컬럼사이즈 재조정
    }
        break;
    case Chat_LogOut:                                                           // client보낸 프로토콜이 Chat_LogOut 이라면
        if (finditem.count() != 0) {                                            // clientTreeWidget에서 찾은 아이템이 하나이상 있다면
            foreach(auto item, finditem) {                                      // 찾은 item iterator
                if(item->text(0) != "X" && item->text(0) != "-b" &&             // item의 0번째 text가 "X" && "-b" && "Ob가 아니라면
                        item->text(0) != "Ob") {
                    item->setText(0, "X");                                      // item의 0번째 text를 "X"로 세팅
                } else {
                    ui->clientTreeWidget->takeTopLevelItem(ui->clientTreeWidget // clietnTreeWidget에서 현재 item 제거
                                                           ->indexOfTopLevelItem(item));
                    ui->clientTreeWidget->update();                             // clientTreeiwdget 업데이트
                }
                clientList.removeOne(clientConnection);                         // clientList에 등록된 tcpsocket 객체 제거
                clientSocketHash.remove(name);                                  // clientsockethash에 현재 name에 맞는 socket 제거
                portSocketHash.remove(port);                                    // 현재 서버에 접속된 port에대한 소켓 Data 삭제
            }
        } else {
            foreach(auto item, findchatitem) {                                  // finditem이 0이라면 findchatitem에서 항목가져와
                saveitem = item->clone();                                       // 임시저장용 아이템에 아이템복사
                if(item->text(0) != "X" && item->text(0) != "-b"                // 현제상태 초기화
                        && item->text(0) != "Ob") {
                    item->setText(0, "X");                                      // 회원이면 X로 표시
                    saveitem = item->clone();                                   // 임시저장용 아이템에 아이템복사
                    ui->chatTreeWidget->takeTopLevelItem(ui->chatTreeWidget     // chatTreeWidget에 항목삭제
                                                         ->indexOfTopLevelItem(item));
                    ui->chatTreeWidget->update();                               // chatTreeWidget 업데이트
                    ui->clientTreeWidget->addTopLevelItem(saveitem);            // clientTreeWidget에 항목추가
                } else {                                                        // 비회원이라면
                    ui->chatTreeWidget->takeTopLevelItem(ui->chatTreeWidget     // chatTreeWidget에서 item삭제
                                                         ->indexOfTopLevelItem(item));
                    ui->chatTreeWidget->update();                               // chatTreeWidget 업데이트
                }
                clientList.removeOne(clientConnection);                         // QList<QTcpSocket*> clientList;
                clientSocketHash.remove(name);                                  // LogOut시 서버에 접속된 소켓 Data 삭제
                portSocketHash.remove(port);                                    // portSocketHash의 port삭제
            }
        }
        break;
    }
}

void ChatServer::removeClient()                                                 // 클라이언트가 서버를 나갈때 함수
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));       // 서버를 나간 클라이언트 소켓 을 sender로 받아옴
    clientList.removeOne(clientConnection);                                     // clientList에서 소켓삭제
    clientConnection->deleteLater();                                            // 소켓이 삭제될때 발동
    QString name = clientNameHash[clientConnection->peerPort()];                // clientNameHash에서 name추출
    auto finditem = ui->clientTreeWidget->findItems(name, Qt::MatchContains, 1);// clientTreeWidget원하는 name추출
    foreach(auto item, finditem) {                                              // finditem에 item검색
        if (finditem.count() != 0) {
            item->setText(0, "X");                                              // 현재상태를 X로 변경
        }
    }

    clientList.removeOne(clientConnection);                                     // clientList에서 소켓삭제
    clientConnection->deleteLater();                                            // 소켓이 삭제될때 발동
}

void ChatServer::addClient(int id, QString name)                                // clientmanager에서 등록or modify시 addclient slot발생
{
    clientIDList << id;                                                         // addclient에 받아온 id를 clientIdList에 등록
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->clientTreeWidget);          // ClientTreewidget에 선택한 아이템을 객체로 복사
    item->setText(0, "X");                                                      // 현재상태 X로 초기화
    item->setText(1, name);                                                     // name 세팅
    ui->clientTreeWidget->addTopLevelItem(item);                                // clientTreeWidget에 항목추가
    clientIDHash[name] = id;                                                    // clientIDHash에 이름에따른 id추가
    ui->clientTreeWidget->resizeColumnToContents(0);                            // clientTreeWidget에 항목 추가시 자등으로 column정렬
}

int ChatServer::makeNonId()                                                     // 비회원 아이디 등록
{
    if(clientIDList.size( ) == 0) {                                             // clientList에 등록이 안됫다면
        return 10000;                                                           // 10000번부터 id부여
    } else {                                                                    // clientList에 Id가 하나라도 잇다면
        auto id = clientIDList.back();                                          // clientList의 마지막값을가져와
        return ++id;                                                            // +1된 값으로 id 지정
    }
}

void ChatServer::NonMemaddClient(QString name)                                  // 비회원 이름등록
{
    int id = makeNonId();                                                       // 비회원 ID등록
    clientIDList << id;                                                         // clientIDList에 id등록
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->clientTreeWidget);          // clientTreeWidget에 item가져와 객체생성
    item->setText(0, "-b");                                                     // 비회원 처음등록후 -b상태 초기화
    item->setText(1, name);                                                     // 비회원 이름 초기화
    ui->clientTreeWidget->addTopLevelItem(item);                                // clientTreeWidget에 항목추가
    clientIDHash[name] = id;                                                    // clientIDHash에 이름에따른 id 추가
    ui->clientTreeWidget->resizeColumnToContents(0);                            // clientTreeWidget에 항목 추가시 자등으로 column정렬
}

void ChatServer::kickOut()                                                      // 클라이언트 강퇴
{
    if(ui->clientTreeWidget->currentItem() == nullptr) {                        // 선택된클라이언트가 없을시 리턴
        return;
    }

    if(ui->chatTreeWidget->topLevelItemCount()) {                               // chatTreeWidget에 항목이 존재한다면
        QString name = ui->chatTreeWidget->currentItem()->text(1);              // 선택된 항목의 이름을 name으로 등록
        QTcpSocket* sock = clientSocketHash[name];                              // sockethash에 이름에맞는 socket 데이터를 sock에 저장
        quint16 port = sock->peerPort();                                        // 소켓에포트를 등록
        QByteArray sendArray;                                                   // 보낼ByteArray 생성
        QDataStream out(&sendArray, QIODevice::WriteOnly);                      // 데이터 출력스트림 생성
        out << Chat_KickOut;                                                    // 첫번째 인자로 type입력
        out.writeRawData("", 1020);                                             // type data인 4비트 제외 1020데이터에 빈내용 입력

        sock->write(sendArray);                                                 // 소켓에 데이터를 써서 보냄

        auto findchatitem = ui->chatTreeWidget->findItems(name,                 // chatTreeWidget에서 원하는 name 찾기
                                                          Qt::MatchFixedString, 1);
        QTreeWidgetItem* saveitem;                                              // 임시등록용 item객체생성
        foreach (auto item, findchatitem) {                                     // 선택된 아이템 등록
            if(item->text(0) != "-" && item->text(0) != "Ob") {                 // 상태 초기화
                item->setText(0, "-");                                          // 회원이라면 -
            } else {
                item->setText(0, "-b");                                         // 비회원이라면 -b
            }
            saveitem = item->clone();                                           // item항목 복사
            clientNameHash.remove(port);                                        // clientNameHash에 port제거
            ui->chatTreeWidget->takeTopLevelItem(ui->chatTreeWidget             // chatTreeWidget 의 선택한 item 제거
                                                 ->indexOfTopLevelItem(item));
            ui->chatTreeWidget->update();                                       // chatTreeWidget 업데이트
        }
        ui->clientTreeWidget->addTopLevelItem(saveitem);                        // 복사된 item항목 clientTreeWidget에 등록
        ui->clientTreeWidget->resizeColumnToContents(0);                        // clientTreeWidget에 항목 추가시 자등으로 column정렬
    }
}

void ChatServer::inviteClient()                                                 // 클라이언트 초대
{
    if(ui->clientTreeWidget->currentItem() == nullptr ||                        // clientTreeWidget에 선택된아이템이 없다면
            ui->clientTreeWidget->currentItem()->text(0) == "X") {
        return;
    }

    if(ui->clientTreeWidget->topLevelItemCount()) {                             // clientTreeWidget에 아이템항목이 하나라도 잇다면
        QString name = ui->clientTreeWidget->currentItem()->text(1);            // clientTreeWidget의 이름을 가져와 등록
        QByteArray sendArray;                                                   // 데이터 출력용 변수선언
        QDataStream out(&sendArray, QIODevice::WriteOnly);                      // 데이터 출력용 스트림 생성
        out << Chat_Invite;                                                     // Chat_Invite의 첫인자로 입력
        out.writeRawData("", 1020);                                             // 나머지 1020비트 에 빈데이터 입력
        QTcpSocket* sock = clientSocketHash[name];                              // clientSocketHash에 이름으로 등록된 소켓가져옴
        quint16 port = sock->peerPort();                                        // 소켓에서 포트추출
        sock->write(sendArray);                                                 // iterator로 비교안해도 소켓에 바로쓰면됨
        auto finditem = ui->clientTreeWidget->findItems(name,                   // clientTreeWidget에 원하는 item찾기
                                                        Qt::MatchFixedString, 1);
        QTreeWidgetItem* saveitem = new QTreeWidgetItem;                        // 임시저장용 item 객체생성
        foreach (auto item, finditem) {                                         // 찾은 item정보 추출
            if(item->text(0) != "O" && item->text(0) != "-b") {                 // 상태 초기화
                item->setText(0, "O");                                          // 회원이면 O상태
            } else {
                item->setText(0, "Ob");                                         // 비회원이면 Ob상태
            }
            saveitem = item->clone();                                           // 임시저장객체에 복사
            ui->clientTreeWidget->takeTopLevelItem(ui->clientTreeWidget         // clientTreeWidget에 선택item 제거
                                                   ->indexOfTopLevelItem(item));
            ui->clientTreeWidget->update();                                     // clientTreeWidget항목 업데이트
        }
        clientNameHash[port] = name;                                            // clientNameHash에 포트로 이름등록
        ui->chatTreeWidget->addTopLevelItem(saveitem);                          // 임시생성item을 chatTreeWidget에 등록
        ui->chatTreeWidget->resizeColumnToContents(0);                          // clientNameHash에 항목 추가시 자등으로 column정렬
    }
}

void ChatServer::on_clientTreeWidget_customContextMenuRequested(const QPoint &pos)
{
    if(ui->clientTreeWidget->currentItem() == nullptr) {
        return;
    }

    QPoint globalPos = ui->clientTreeWidget->mapToGlobal(pos);
    if(ui->clientTreeWidget->indexAt(pos).isValid()){
        menu->exec(globalPos);
    }
}

void ChatServer::on_chatTreeWidget_customContextMenuRequested(const QPoint &pos)
{
    if(ui->chatTreeWidget->currentItem() == nullptr){
        return;
    }

    QPoint globalPos = ui->chatTreeWidget->mapToGlobal(pos);
    if(ui->chatTreeWidget->indexAt(pos).isValid()){
        chatmenu->exec(globalPos);
    }
}

void ChatServer::acceptConnection()
{
    qDebug("Connected, preparing to receive files!");

    QTcpSocket* receivedSocket = fileServer->nextPendingConnection();
    connect(receivedSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
}

void ChatServer::readClient()
{
    qDebug("Receiving file ...");
    QTcpSocket* receivedSocket = dynamic_cast<QTcpSocket *>(sender( ));
    QString filename, name;

    if (byteReceived == 0) { // just started to receive data, this data is file information
        progressDialog->reset();
        progressDialog->show();

        QString ip = receivedSocket->peerAddress().toString();
        quint16 port = receivedSocket->peerPort();
        qDebug() << ip << " : " << port;

        QDataStream in(receivedSocket);
        in >> totalSize >> byteReceived >> filename >> name;
        progressDialog->setMaximum(totalSize);

        QTreeWidgetItem* item = new QTreeWidgetItem(ui->messageTreeWidget);
        item->setText(0, ip);
        item->setText(1, QString::number(port));
        item->setText(2, QString::number(clientIDHash[name]));
        item->setText(3, name);
        item->setText(4, filename);
        item->setText(5, QDateTime::currentDateTime().toString());
        item->setToolTip(4, filename);

        for(int i = 0; i < ui->messageTreeWidget->columnCount(); i++)
            ui->messageTreeWidget->resizeColumnToContents(i);

        ui->messageTreeWidget->addTopLevelItem(item);
        logThread->appendData(item);

        QFileInfo info(filename);
        QString currentFileName = info.fileName();
        file = new QFile(currentFileName);
        file->open(QFile::WriteOnly);
    } else { // Officially read the file content
        inBlock = receivedSocket->readAll();

        byteReceived += inBlock.size();
        file->write(inBlock);
        file->flush();
    }

    progressDialog->setValue(byteReceived);

    if (byteReceived == totalSize) {
        qDebug() << QString("%1 receive completed").arg(filename);

        inBlock.clear();
        byteReceived = 0;
        totalSize = 0;
        progressDialog->reset();
        progressDialog->hide();
        file->close();
        delete file;
    }
}

void ChatServer::privateChat()
{
    QString cname = ui->chatTreeWidget->currentItem()->text(1);
    QTcpSocket* sock = clientSocketHash[cname];
    quint16 port = sock->peerPort();
    ChatServerAdmin* Admin = new ChatServerAdmin(cname,port);
    clientWindowHash[cname] = Admin;
    connect(Admin,SIGNAL(sendData(QString,QString)), SLOT(privateChatSend(QString,QString)));

    Admin->setWindowTitle("Prvate Chat");
    Admin->show();
}

void ChatServer::privateChatSend(QString name, QString str)
{
    QTcpSocket* sock = clientSocketHash[name];
    quint16 port = sock->peerPort();
    QByteArray sendArray;
    sendArray.clear();
    QDataStream out(&sendArray, QIODevice::WriteOnly);
    out << Chat_Talk;
    sendArray.append("<font color=blue>");
    sendArray.append("Admin");
    sendArray.append("</font> : ");
    sendArray.append(str.toStdString().data());
    sock->write(sendArray);


    QTreeWidgetItem *logitem = new QTreeWidgetItem(ui->messageTreeWidget);
    logitem->setText(1, "Server");
    logitem->setText(2, "Private");
    logitem->setText(3, "Admin");
    logitem->setText(4, str);
    logitem->setText(5, QDateTime::currentDateTime().toString());
    logitem->setToolTip(4, str);

    for(int i = 0; i < ui->messageTreeWidget->columnCount(); i++)
        ui->messageTreeWidget->resizeColumnToContents(i);

    ui->messageTreeWidget->addTopLevelItem(logitem);
}

void ChatServer::on_noticepushButton_clicked()
{
    if (clientNameHash.count() == 0) {
        QMessageBox::critical(this, tr("Notice Details"), \
                              tr("There isn't anybody in the ChatList."));
        return;
    }

    ui->noticepushButton->setEnabled(false);
    notice = new ChatNoticeDetails;
    notice->setClientList(clientNameHash);

    connect(notice, SIGNAL(sendNoticeData(QString)), SLOT(noticeChatSend(QString)));
    connect(notice, &ChatNoticeDetails::closeNotice, this,
            [this](){ ui->noticepushButton->setEnabled(true); });
    notice->setWindowTitle("Notice Chat");
    notice->show();
}

void ChatServer::noticeChatSend(QString str)
{
    foreach (auto name, clientNameHash) {
        QTcpSocket* sock = clientSocketHash[name];
        quint16 port = sock->peerPort();
        QByteArray sendArray;
        sendArray.clear();
        QDataStream out(&sendArray, QIODevice::WriteOnly);
        out << Chat_Notice;
        sendArray.append("<font color=green>");
        sendArray.append("***Notice***");
        sendArray.append("</font> : ");
        sendArray.append(str.toStdString().data());
        sock->write(sendArray);
    }
    QTreeWidgetItem *logitem = new QTreeWidgetItem(ui->messageTreeWidget);
    logitem->setText(1, "Server");
    logitem->setText(2, "Notice");
    logitem->setText(3, "Admin");
    logitem->setText(4, str);
    logitem->setText(5, QDateTime::currentDateTime().toString());
    logitem->setToolTip(4, str);

    for(int i = 0; i < ui->messageTreeWidget->columnCount(); i++)
        ui->messageTreeWidget->resizeColumnToContents(i);

    ui->messageTreeWidget->addTopLevelItem(logitem);
}
