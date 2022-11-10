#include "clientmanagerform.h"
#include "ui_clientmanagerform.h"
#include "clientitem.h"

#include <QFile>
#include <QMenu>

//ClientManager생성자
ClientManagerForm::ClientManagerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;                                                           // TreeWidget과 ToolBox간 거리 조절용
    sizes << 320 << 600;                                                        // 사이즈 지정
    ui->splitter->setSizes(sizes);                                              // 사이즈 세팅

    QAction* removeAction = new QAction(tr("&Remove"));                         // Remove Action 생성
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));             // Remove Action 동작시 removeItem슬롯 실행

    menu = new QMenu;                                                           // Action을 동작하기위한 메뉴생성 멤버변수
    menu->addAction(removeAction);                                              // 액션 추가
    ui->clienttreeView->setContextMenuPolicy(Qt::CustomContextMenu);          // 위젯의 ContextMenu의 속성을 표시하기 위함
    connect(ui->clienttreeView, SIGNAL(customContextMenuRequested(QPoint)),   // clienttreeWidget내부에서 우클릭동작시 ContextMenu를 실행하기 위함
            this, SLOT(showContextMenu(QPoint)));
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),                        // searchLineEdit의 returnPressed동작시 searchPushButton슬롯 실행
            this, SLOT(on_searchPushButton_clicked()));
}

void ClientManagerForm::loadData()                                              // 프로그램 실행시 clientlist값을 불러오기 위함
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "clientitemConnection");
    db.setDatabaseName("clientitem.db");
    if (db.open()) {
        QSqlQuery query(db);
        query.exec("CREATE TABLE IF NOT EXISTS clientitem("
                   "c_id INTEGER Primary Key, "
                   "c_name VARCHAR(30) NOT NULL, "
                   "c_phon VARCHAR(20) NOT NULL, "
                   "c_addr VARCHAR(50));");

        clientModel = new QSqlTableModel(this, db);
        clientModel->setTable("clientitem");
        clientModel->select();
        clientModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
        clientModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
        clientModel->setHeaderData(2, Qt::Horizontal, tr("Phone Number"));
        clientModel->setHeaderData(3, Qt::Horizontal, tr("Address"));

        sclientModel = new QSqlTableModel(this, db);
        sclientModel->QSqlQueryModel::setQuery(QString("select * from clientitem where c_id = 0"),db);
        sclientModel->select();
        sclientModel->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
        sclientModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));
        sclientModel->setHeaderData(2, Qt::Horizontal, QObject::tr("PhonNumber"));
        sclientModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Address"));

        ui->clienttreeView->setModel(clientModel);
        ui->searchTreeView->setModel(sclientModel);

        ui->clienttreeView->setRootIsDecorated(false);
        ui->searchTreeView->setRootIsDecorated(false);

//        for(int i=0;i<clientModel->columnCount();i++){
//            ui->searchTreeView->resizeColumnToContents(i);
//        }
    }

    //    for(int i = 0; i < clientModel->rowCount(); i++) {
    //        int id = clientModel->data(clientModel->index(i, 0)).toInt();
    //        QString name = clientModel->data(clientModel->index(i, 1)).toString();
    //        emit clientAdded(id, name);
    //    }
}

ClientManagerForm::~ClientManagerForm()                                         // ClientManagerForm 소멸자
{
    /*delete ui;                                                                  // ui 종료

    QFile file("clientlist.txt");                                               // 파일명 설정
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))                     // 파일저장이 안된다면 리턴
        return;

    QTextStream out(&file);                                                     // 파일출력용 스트림 생성
    for (const auto& v : clientList) {                                          // clientList데이터를 저장 반복
        ClientItem* c = v;                                                      // clientList의 항목을 각각 c에 저장
        out << c->id() << "," << c->getName() << ",";                         // id, name, number, address형으로 저장후 다시 반복
        out << c->getPhoneNumber() << ",";
        out << c->getAddress() << "\n";
    }
    file.close( );*/                                                              // 파일 Close
    delete ui;
    QSqlDatabase db = QSqlDatabase::database("clientitemConnection");
    if(db.isOpen()) {
        clientModel->submitAll();
        db.close();
    }
}

int ClientManagerForm::makeId( )                                                // Client Id를 부여하기위한 멤버함수
{
    if(clientModel->rowCount() == 0) {                                               // clientList의 데이터가 없다면
        return 1000;                                                            // id를 1000번부터 부여
    } else {
        int lastNum = clientModel->rowCount();
        auto id = clientModel->data(clientModel->index(lastNum-1, 0)).toInt();             // clientList의 마지막값을 id로 가져와
        return ++id;                                                            // +1 하여 아이디 부여
    }
}

void ClientManagerForm::removeItem()                                            // treewidget의 항목 제거용
{
    QModelIndex model = ui->clienttreeView->currentIndex();                     // 선택된 항목을 저장
    QSqlDatabase db = QSqlDatabase::database("clientitemConnection");
    if(db.isOpen() && model.isValid()) {
        //clientList.remove(clientModel->data(index.siblingAtColumn(0)).toInt());
        clientModel->removeRow(model.row());
        clientModel->select();
//        for(int i=0;i<clientModel->columnCount();i++){
//            ui->clienttreeView->resizeColumnToContents(i);
//        }
    }
}

void ClientManagerForm::showContextMenu(const QPoint &pos)                      // treewidget내에서 우클릭시 action을 사용하기 위함
{
    QPoint globalPos = ui->clienttreeView->mapToGlobal(pos);                  // 우클릭된 모니터의 좌표값 불러오기
    if(ui->clienttreeView->indexAt(pos).isValid())
        menu->exec(globalPos);
    // 좌표값기준으로 menu 실행
}

void ClientManagerForm::on_searchPushButton_clicked()                           // Search버튼 - Item검색
{
    //    ui->searchTreeWidget->clear();                                              // searchTreeWidget 업데이트
    //    //    for(int i = 0; i < ui->treeWidget->columnCount(); i++)
    //    int i = ui->searchComboBox->currentIndex();                                 // comboBox의 최신값 저장
    //    auto flag = (i)? Qt::MatchCaseSensitive|Qt::MatchContains                   // flag 세부사항 설정
    //                   : Qt::MatchCaseSensitive;
    //    {
    //        auto items = ui->clienttreeWidget->findItems(ui->searchLineEdit         // flag에 맞도록 searchLineEdit의 항목을 찾아 item저장
    //                                                     ->text(), flag, i);

    //        foreach(auto i, items) {                                                // 찾은 item항목의 내부값들을 저장
    //            ClientItem* c = static_cast<ClientItem*>(i);                        // 최신선택항목 item값 저장
    //            int id = c->id();                                                   // 내부값에서 id얻기
    //            QString name = c->getName();                                        // 내부값에서 name얻기
    //            QString number = c->getPhoneNumber();                               // 내부값에서 phonnumber얻기
    //            QString address = c->getAddress();                                  // 내부값에서 address얻기
    //            ClientItem* item = new ClientItem(id, name, number, address);       // 뽑아저 저장한 항목들을 새로운 아이템으로 저장
    //            ui->searchTreeWidget->addTopLevelItem(item);                        // searchTreeWidget에 항목으로 추가
    //        }
    //    }
    int i = ui->searchComboBox->currentIndex();                       // 검색할 id,이름,전화번호, 타입을 설정
    QString sch = ui->searchLineEdit->text();
    QSqlDatabase db = QSqlDatabase::database("clientitemConnection");

    switch (i) {
    case 0:
        sclientModel->QSqlQueryModel::setQuery(QString("select * "
                                                       "from clientitem "
                                                       "where c_id = %1").arg(sch),db);
        for(int i=0;i<clientModel->columnCount();i++){
            ui->searchTreeView->resizeColumnToContents(i);
        }
        break;
    case 1:
        sclientModel->QSqlQueryModel::setQuery(QString("select * from clientitem where c_name like '%%1%' order by c_id").arg(sch),db);
        for(int i=0;i<clientModel->columnCount();i++){
            ui->searchTreeView->resizeColumnToContents(i);
        }
        break;
    case 2:
        sclientModel->QSqlQueryModel::setQuery(QString("select * from clientitem where c_phon like '%%1%' order by c_id").arg(sch),db);
        for(int i=0;i<clientModel->columnCount();i++){
            ui->searchTreeView->resizeColumnToContents(i);
        }
        break;
    case 3:
        sclientModel->QSqlQueryModel::setQuery(QString("select * from clientitem where c_addr like '%%1%' order by c_id").arg(sch),db);
        for(int i=0;i<clientModel->columnCount();i++){
            ui->searchTreeView->resizeColumnToContents(i);
        }
        break;
    default:
        break;
    }
}

void ClientManagerForm::on_modifyPushButton_clicked()                           // Modify버튼 - Item수정
{
// clienttreeWidget에서 최신선택항목 가져오기
// item항목이 비어있지않다면
    QModelIndex model = ui->clienttreeView->currentIndex();
    QSqlDatabase db = QSqlDatabase::database("clientitemConnection");
    if(db.isOpen()){
        int key = model.sibling(model.row(),0).data().toInt();                                        // 0번째항목에서 key값추출
        //ClientItem* c = clientList[key];                                        // clientList에 key값을 이용해 아이템 추출
        QString name, number, address;                                          // 이름,전화번호,주소 QString형 변수 생성
        name = ui->nameLineEdit->text();                                        // nameLineEdit의 text를 name변수에 저장
        number = ui->phoneNumberLineEdit->text();                               // phoneNumberLineEdit의 text를 number변수에 저장
        address = ui->addressLineEdit->text();                                  // addressLineEdit의 text를 address변수에 저장
        /*c->setName(name);                                                       // item에 name멤버변수저장
            c->setPhoneNumber(number);                                              // item에 number멤버변수저장
            c->setAddress(address);                                                 // item에 address멤버변수저장
            clientList[key] = c;*/                                                    // clientList의 key값에 item인 c 저장

        clientModel->setData(model.siblingAtColumn(1), name);
        clientModel->setData(model.siblingAtColumn(2), number);
        clientModel->setData(model.siblingAtColumn(3), address);
        clientModel->submit();

//        for(int i=0;i<clientModel->columnCount();i++){
//            ui->clienttreeView->resizeColumnToContents(i);
//        }
    }
}

void ClientManagerForm::on_addPushButton_clicked()                              // Add버튼 - Item추가
{
    QString name, number, address;                                              // 이름,전화번호,주소 QString형 변수 생성
    int id = makeId();                                                         // id생성후 id변수에 저장
    name = ui->nameLineEdit->text();                                            // nameLineEdit에 text를 name변수에 저장
    number = ui->phoneNumberLineEdit->text();                                   // phoneNumberLineEdit의 text를 number변수에 저장
    address = ui->addressLineEdit->text();                                      // addressLineEdit의 text를 address변수에 저장
    QSqlDatabase db = QSqlDatabase::database("clientitemConnection");
    if(db.isOpen() && name.length()) {
        QSqlQuery query(clientModel->database());
        query.prepare("INSERT INTO clientitem VALUES (?, ?, ?, ?)");
        query.bindValue(0, id);
        query.bindValue(1, name);
        query.bindValue(2, number);
        query.bindValue(3, address);
        query.exec();
        clientModel->select();
//        for(int i=0;i<clientModel->columnCount();i++){
//            ui->clienttreeView->resizeColumnToContents(i);
//        }
        //emit clientAdded(id, name);
    }
}

void ClientManagerForm::on_deletePushButton_clicked()                           // Delete버튼 - Item삭제
{
    removeItem();                                                               // removeItem() 멤버함수 호출
}

void ClientManagerForm::clientIdListData(int index)                             // ClientComoboBox의 Index에 따른 ClientList를 가져오기 위함
{
    //ui->searchTreeWidget->clear();                                              // searchTreeWidget 항목클리어
    QString Idstr;                                                              // Id와 이름을 QString형으로 같이 저장하기위한 변수
    QList<QString> IdList;                                                      // Idstr인 QString을 저장하기위한 QList 변수

    for(int i = 0; i < clientModel->rowCount(); i++) {
        QString id = clientModel->data(clientModel->index(i, 0)).toString();
        QString name = clientModel->data(clientModel->index(i, 1)).toString();
        Idstr = QString("%1, %2").arg(id).arg(name);                    // "ID, 이름" 형태의 QString형 변수 저장
        IdList.append(Idstr);
    }

//    for (const auto& v : clientList) {                                          // clientList에 모든 항목들을 반복
//        ClientItem* c = v;                                                      // 뽑은 아이템을 ClientItem변수에 저장
//        sentid = c->id();                                                       // 뽑은 id값을 sentid변수에 저장
//        sentname = c->getName();                                                // 뽑은 name값을 sentname값변수에 저장

//        Idstr = QString("%1, %2").arg(sentid).arg(sentname);                    // "ID, 이름" 형태의 QString형 변수 저장
//        IdList.append(Idstr);                                                   // QList에 위에서 계산된 QString 항목 저장
//    }

    emit clientDataListSent(IdList);                                            // ClientList를 OrderForm에 전달하기위한 시그널
}

void ClientManagerForm::clientNameListData(QString name)                        // ClientComboBox의 name설정시 이름에따른 ClientList 가져오기 위함
{
    //ui->searchTreeWidget->clear();                                              // searchTreeWidget 항목클리어

//    auto items = ui->clienttreeWidget->findItems(name,Qt::MatchCaseSensitive    // clienttreeWidget에 name값 찾기
//                                                 |Qt::MatchContains,1);
    QSqlDatabase db = QSqlDatabase::database("clientitemConnection");
    sclientModel->QSqlQueryModel::setQuery(QString("select * "
                                                   "from clientitem "
                                                   "where c_name "
                                                   "like '%%1%' "
                                                   "order by c_id").arg(name),db);

    for(int i = 0; i < sclientModel->rowCount(); i++) {
        int id = sclientModel->data(clientModel->index(i, 0)).toInt();
        QString name = sclientModel->data(clientModel->index(i, 1)).toString();
        QString number = sclientModel->data(clientModel->index(i, 2)).toString();
        QString address = sclientModel->data(clientModel->index(i, 3)).toString();
        emit clientFindDataSent(id,name,number,address);
    }
}


void ClientManagerForm::on_clienttreeView_clicked(const QModelIndex &index)             // TreeWidget의고객창 선택버튼 - 고객메뉴로 선택됨
{
    Q_UNUSED(index);                                                                // column인자 사용안함
    QModelIndex model = ui->clienttreeView->currentIndex();
    ui->idLineEdit->setText(model.sibling(model.row(),0).data().toString());        // idLineEdit에 item의 text(0)값을 저장
    ui->nameLineEdit->setText(model.sibling(model.row(),1).data().toString());                                   // nameLineEdit에 item의 text(1)값을 저장
    ui->phoneNumberLineEdit->setText(model.sibling(model.row(),2).data().toString());                            // phoneNumberLineEdit item의 text(2)값을 저장
    ui->addressLineEdit->setText(model.sibling(model.row(),3).data().toString());                                // addressLineEdit에 item의 text(3)값을 저장
    ui->toolBox->setCurrentIndex(0);                                                 // toolbox화면상태 0번으로 변경
}


void ClientManagerForm::on_searchTreeView_doubleClicked(const QModelIndex &index)   // TreeWidget의검색창 선택버튼 - 검색메뉴로 선택됨
{
    Q_UNUSED(index);                                                           // column인자 사용안함
    QModelIndex model = ui->searchTreeView->currentIndex();
    ui->idLineEdit->setText(model.sibling(model.row(),0).data().toString());                                     // idLineEdit에 item의 text(0)값을 저장
    ui->nameLineEdit->setText(model.sibling(model.row(),1).data().toString());                                   // nameLineEdit에 item의 text(1)값을 저장
    ui->phoneNumberLineEdit->setText(model.sibling(model.row(),2).data().toString());                            // phoneNumberLineEdit item의 text(2)값을 저장
    ui->addressLineEdit->setText(model.sibling(model.row(),3).data().toString());                                // addressLineEdit에 item의 text(3)값을 저장
    ui->toolBox->setCurrentIndex(0);                                            // toolbox화면상태 0번으로 변경
}

