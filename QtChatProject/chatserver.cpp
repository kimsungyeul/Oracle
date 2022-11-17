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
    case Chat_Login: {                                                          // client보낸 프로토콜이 Chat_Login 라면
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

    case Chat_In: {                                                             // client보낸 프로토콜이 Chat_In 라면
        foreach(auto item, finditem) {                                          // 등록된 이름들에서 finditem iterator
            if(item->text(0) != "O" && item->text(0) != "-b") {
                item->setText(0, "O");
            } else {
                item->setText(0, "Ob");
            }
            saveitem = item->clone();
            clientNameHash[port] = name;            // 채팅입장시 Name Hash포트에 포트에따른 이름저장
            ui->clientTreeWidget->takeTopLevelItem(ui->clientTreeWidget
                                                   ->indexOfTopLevelItem(item));
            ui->clientTreeWidget->update();
        }
        ui->chatTreeWidget->addTopLevelItem(saveitem);

        ui->chatTreeWidget->resizeColumnToContents(0);
    }
        break;
    case Chat_Talk: {
        foreach(QTcpSocket *sock, clientList) {
            if(clientNameHash.contains(sock->peerPort()) &&
                    sock != clientConnection){          // clientList에 포함된 포트만 채팅되도록 설정
                QByteArray sendArray;
                sendArray.clear();
                QDataStream out(&sendArray, QIODevice::WriteOnly);
                out << Chat_Talk;
                sendArray.append("<font color=lightsteelblue>");
                sendArray.append(clientNameHash[port].toStdString().data());
                sendArray.append("</font> : ");
                sendArray.append(name.toStdString().data());
                sock->write(sendArray);
                qDebug() << sock->peerPort();
            }
        }

        if (clientWindowHash.count() != 0) {
            clientWindowHash[clientNameHash[port]]
                    ->clientChatRecv(clientNameHash[port],data);
        }

        QTreeWidgetItem *logitem = new QTreeWidgetItem(ui->messageTreeWidget);
        logitem->setText(0, ip);
        logitem->setText(1, QString::number(port));
        logitem->setText(2, QString::number(clientIDHash[clientNameHash[port]]));
        logitem->setText(3, clientNameHash[port]);
        logitem->setText(4, QString(data));
        logitem->setText(5, QDateTime::currentDateTime().toString());
        logitem->setToolTip(4, QString(data));

        for(int i = 0; i < ui->messageTreeWidget->columnCount(); i++)
            ui->messageTreeWidget->resizeColumnToContents(i);

        ui->messageTreeWidget->addTopLevelItem(logitem);

        logThread->appendData(logitem);
    }
        break;
    case Chat_Out: {
        foreach(auto item, findchatitem) {
            if(item->text(0) != "-" && item->text(0) != "Ob") {
                item->setText(0, "-");
            } else {
                item->setText(0, "-b");
            }
            saveitem = item->clone();
            clientNameHash.remove(port);
            ui->chatTreeWidget->takeTopLevelItem(ui->chatTreeWidget
                                                 ->indexOfTopLevelItem(item));
            ui->chatTreeWidget->update();
        }
        ui->clientTreeWidget->addTopLevelItem(saveitem);
        ui->clientTreeWidget->resizeColumnToContents(0);
    }
        break;
    case Chat_LogOut:
        if (finditem.count() != 0) {
            foreach(auto item, finditem) {
                if(item->text(0) != "X" && item->text(0) != "-b" &&
                        item->text(0) != "Ob") {
                    item->setText(0, "X");
                } else {
                    ui->clientTreeWidget->takeTopLevelItem(ui->clientTreeWidget
                                                           ->indexOfTopLevelItem(item));
                    ui->clientTreeWidget->update();
                }
                clientList.removeOne(clientConnection);                                     // QList<QTcpSocket*> clientList;
                clientSocketHash.remove(name);                                              // LogOut시 서버에 접속된 소켓 Data 삭제
                portSocketHash.remove(port);                                                // 현재 서버에 접속된 port에대한 소켓 Data 삭제
            }
        } else {
            foreach(auto item, findchatitem) {
                saveitem = item->clone();
                if(item->text(0) != "X" && item->text(0) != "-b" && item->text(0) != "Ob") {
                    item->setText(0, "X");
                    saveitem = item->clone();
                    ui->chatTreeWidget->takeTopLevelItem(ui->chatTreeWidget
                                                         ->indexOfTopLevelItem(item));
                    ui->chatTreeWidget->update();
                    ui->clientTreeWidget->addTopLevelItem(saveitem);
                } else {
                    ui->chatTreeWidget->takeTopLevelItem(ui->chatTreeWidget
                                                         ->indexOfTopLevelItem(item));
                    ui->chatTreeWidget->update();
                }
                clientList.removeOne(clientConnection);                                         // QList<QTcpSocket*> clientList;
                clientSocketHash.remove(name);                                                  // LogOut시 서버에 접속된 소켓 Data 삭제
                portSocketHash.remove(port);                                                    // 현재 서버에 접속된 port에대한 소켓 Data 삭제
            }
        }
        break;
    }
}

void ChatServer::removeClient()
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));
    clientList.removeOne(clientConnection);
    clientConnection->deleteLater();
    QString name = clientNameHash[clientConnection->peerPort()];
    auto finditem = ui->clientTreeWidget->findItems(name, Qt::MatchContains, 1);
    foreach(auto item, finditem) {
        if (finditem.count() != 0) {
            item->setText(0, "X");
        }
    }

    clientList.removeOne(clientConnection);
    clientConnection->deleteLater();
}

void ChatServer::addClient(int id, QString name)
{
    clientIDList << id;
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->clientTreeWidget);
    item->setText(0, "X");
    item->setText(1, name);
    ui->clientTreeWidget->addTopLevelItem(item);
    clientIDHash[name] = id;
    ui->clientTreeWidget->resizeColumnToContents(0);
}

int ChatServer::makeNonId()
{
    if(clientIDList.size( ) == 0) {
        return 10000;
    } else {
        auto id = clientIDList.back();
        return ++id;
    }
}

void ChatServer::NonMemaddClient(QString name)
{
    int id = makeNonId();
    clientIDList << id;
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->clientTreeWidget);
    item->setText(0, "-b");
    item->setText(1, name);
    ui->clientTreeWidget->addTopLevelItem(item);
    clientIDHash[name] = id;
    ui->clientTreeWidget->resizeColumnToContents(0);
}

void ChatServer::kickOut()
{
    if(ui->clientTreeWidget->currentItem() == nullptr) {
        return;
    }

    if(ui->chatTreeWidget->topLevelItemCount()) {
        QString name = ui->chatTreeWidget->currentItem()->text(1);
        QTcpSocket* sock = clientSocketHash[name];          // sockethash에 이름에맞는 socket 데이터를 sock에 저장
        quint16 port = sock->peerPort();
        QByteArray sendArray;
        QDataStream out(&sendArray, QIODevice::WriteOnly);
        out << Chat_KickOut;
        out.writeRawData("", 1020);

        sock->write(sendArray);                             // 아래코드처럼 궂이 안찾아도 바로 찾은 소켓에 쓰면됨

        auto findchatitem = ui->chatTreeWidget->findItems(name, Qt::MatchFixedString, 1);
        QTreeWidgetItem* saveitem;
        foreach (auto item, findchatitem) {
            if(item->text(0) != "-" && item->text(0) != "Ob") {
                item->setText(0, "-");
            } else {
                item->setText(0, "-b");
            }
            saveitem = item->clone();
            clientNameHash.remove(port);
            ui->chatTreeWidget->takeTopLevelItem(ui->chatTreeWidget
                                                 ->indexOfTopLevelItem(item));
            ui->chatTreeWidget->update();
        }
        ui->clientTreeWidget->addTopLevelItem(saveitem);
        ui->clientTreeWidget->resizeColumnToContents(0);
    }
}

void ChatServer::inviteClient()
{
    if(ui->clientTreeWidget->currentItem() == nullptr ||
            ui->clientTreeWidget->currentItem()->text(0) == "X") {
        return;
    }
    if(ui->clientTreeWidget->topLevelItemCount()) {
        QString name = ui->clientTreeWidget->currentItem()->text(1);
        //quint16 port = clientNameHash.key(name, -1);      //포트비교를위해 선언하였으나 소켓Hash에서 이름검색후 바로 쓰면되므로 삭제

        //chatProtocolType data;
        QByteArray sendArray;
        QDataStream out(&sendArray, QIODevice::WriteOnly);
        out << Chat_Invite;
        out.writeRawData("", 1020);
        QTcpSocket* sock = clientSocketHash[name];
        quint16 port = sock->peerPort();
        sock->write(sendArray);                 // iterator로 비교안해도 소켓에 바로쓰면됨
        auto finditem = ui->clientTreeWidget->findItems(name, Qt::MatchFixedString, 1);
        qDebug() << "5";
        QTreeWidgetItem* saveitem = new QTreeWidgetItem;
        qDebug() << "6";
        foreach (auto item, finditem) {
            if(item->text(0) != "O" && item->text(0) != "-b") {
                item->setText(0, "O");
            } else {
                item->setText(0, "Ob");
            }
            saveitem = item->clone();
            ui->clientTreeWidget->takeTopLevelItem(ui->clientTreeWidget
                                                   ->indexOfTopLevelItem(item));
            ui->clientTreeWidget->update();
        }
        clientNameHash[port] = name;
        ui->chatTreeWidget->addTopLevelItem(saveitem);
        ui->chatTreeWidget->resizeColumnToContents(0);
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
    logitem->setText(1, QString::number(port));
    logitem->setText(2, "Admin");
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
    logitem->setText(1, "Notice");
    logitem->setText(2, "Notice");
    logitem->setText(3, "Admin");
    logitem->setText(4, str);
    logitem->setText(5, QDateTime::currentDateTime().toString());
    logitem->setToolTip(4, str);

    for(int i = 0; i < ui->messageTreeWidget->columnCount(); i++)
        ui->messageTreeWidget->resizeColumnToContents(i);

    ui->messageTreeWidget->addTopLevelItem(logitem);
}
