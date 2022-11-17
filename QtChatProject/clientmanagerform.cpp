#include "clientmanagerform.h"
#include "ui_clientmanagerform.h"

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
    ui->clienttreeView->setContextMenuPolicy(Qt::CustomContextMenu);            // 위젯의 ContextMenu의 속성을 표시하기 위함
    connect(ui->clienttreeView, SIGNAL(customContextMenuRequested(QPoint)),     // clienttreeView내부에서 우클릭동작시 ContextMenu를 실행하기 위함
            this, SLOT(showContextMenu(QPoint)));
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),                        // searchLineEdit의 returnPressed동작시 searchPushButton슬롯 실행
            this, SLOT(on_searchPushButton_clicked()));

    sclientModel = new QStandardItemModel(0,3);                                 // 사용자정의모델 컬럼 크기 지정
    sclientModel->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));          // 0번째 컬럼헤더이름 ID 로 정의
    sclientModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));        // 1번째 컬럼헤더이름 Name 로 정의
    sclientModel->setHeaderData(2, Qt::Horizontal, QObject::tr("PhonNumber"));  // 2번째 컬럼헤더이름 PhonNumber 로 정의
    ui->searchTreeView->setModel(sclientModel);                                 // searchTreeView에대한 모델을 지정
    ui->searchTreeView->setRootIsDecorated(false);                              // 첫항목 텝 제거
}

void ClientManagerForm::loadData()                                              // 프로그램 실행시 clientlist값을 불러오기 위함
{   // QSQLITE를 DB로 사용 및 clientitemConnection으로 DB접속선언
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "clientitemConnection");
    db.setDatabaseName("clientitem.db");                                        // clientitem.db를 db이름으로 설정
    if (db.open()) {                                                            // db가 열리면
        QSqlQuery query(db);                                                    // query사용타켓을 db로 설정
        query.exec("CREATE TABLE IF NOT EXISTS clientitem("
                   "c_id INTEGER Primary Key, "
                   "c_name VARCHAR(30) NOT NULL, "
                   "c_phon VARCHAR(20) NOT NULL, "
                   "c_addr VARCHAR(50));");

        clientModel = new QSqlTableModel(this, db);                             // base모델을 db로 지정
        clientModel->setTable("clientitem");                                    // 테이블명을 clientitem으로 지정
        clientModel->select();                                                  // model의 select문으로 전체검색
        clientModel->setHeaderData(0, Qt::Horizontal, tr("ID"));                // 0번째 컬럼헤더이름 ID로 정의
        clientModel->setHeaderData(1, Qt::Horizontal, tr("Name"));              // 1번째 컬럼헤더이름 Name로 정의
        clientModel->setHeaderData(2, Qt::Horizontal, tr("Phone Number"));      // 2번째 컬럼헤더이름 Phone Number로 정의
        clientModel->setHeaderData(3, Qt::Horizontal, tr("Address"));           // 3번째 컬럼헤더이름 Address로 정의

        ui->clienttreeView->setModel(clientModel);                              // clienttreeView의 타겟모델을 clientModel로 지정
        ui->clienttreeView->setRootIsDecorated(false);                          // 컬럼의 첫항목 공백 제거
    }

    for(int i = 0; i < clientModel->rowCount(); i++) {                          // clientModel의 항목개수대로 iterator
        int id = clientModel->data(clientModel->index(i, 0)).toInt();           // Model의 id값 추출
        QString name = clientModel->data(clientModel->index(i, 1)).toString();  // Model의 name값 추출
        emit clientAdded(id, name);                                             // ChatServer에 보내기위한 시그널생성
    }
}

ClientManagerForm::~ClientManagerForm()                                         // ClientManagerForm 소멸자
{
    delete ui;
    QSqlDatabase db = QSqlDatabase::database("clientitemConnection");           // client데이터베이스 접속
    if(db.isOpen()) {                                                           // db가 열린다면
        clientModel->submitAll();                                               // db값 commitAll
        db.close();                                                             // db 닫기
    }
}

int ClientManagerForm::makeId( )                                                // Client Id를 부여하기위한 멤버함수
{
    if(clientModel->rowCount() == 0) {                                          // clientModel의 데이터가 없다면
        return 1000;                                                            // id를 1000번부터 부여
    } else {
        int lastNum = clientModel->rowCount();                                  // 마지막 아이템개수
        auto id = clientModel->data(clientModel->index(lastNum-1, 0)).toInt();  // clientModel의 마지막값을 id로 가져와
        return ++id;                                                            // +1 하여 아이디 부여
    }
}

void ClientManagerForm::removeItem()                                            // treeView의 항목 제거용
{
    QModelIndex model = ui->clienttreeView->currentIndex();                     // 선택된 항목을 저장
    QSqlDatabase db = QSqlDatabase::database("clientitemConnection");           // client DB접속
    if(db.isOpen() && model.isValid()) {                                        // DB가 열리고 model이 유효하면
        clientModel->removeRow(model.row());                                    // 선택된 model의 row값 제거(item제거)
        clientModel->select();                                                  // 제거후 전체표시(업데이트)
    }
}

void ClientManagerForm::showContextMenu(const QPoint &pos)                      // treewidget내에서 우클릭시 action을 사용하기 위함
{
    QPoint globalPos = ui->clienttreeView->mapToGlobal(pos);                    // 우클릭된 모니터의 좌표값 불러오기
    if(ui->clienttreeView->indexAt(pos).isValid())                              // 선택된 좌표값에 index가 있다면
        menu->exec(globalPos);                                                  // menu Action 실행
}

void ClientManagerForm::on_searchPushButton_clicked()                           // Search버튼 - Item검색
{
    sclientModel->clear();                                                      // searchModel 클리어
    int i = ui->searchComboBox->currentIndex();                                 // 검색할 id,이름,전화번호, 타입을 설정
    auto flag = (i)? Qt::MatchFixedString|Qt::MatchContains                     // flag옵션 설정
                   : Qt::MatchCaseSensitive;
    QModelIndexList indexes = clientModel->match(clientModel->index(0, i),      // text와 flag값에 적용되는 모델의 인덱스를 찾아 저장
                                                 Qt::EditRole,
                                                 ui->searchLineEdit->text(),
                                                 -1, Qt::MatchFlags(flag));
    foreach(auto idx, indexes) {                                                // 찾은 idexes를 찾아 iterator
        int id = clientModel->data(idx.siblingAtColumn(0)).toInt();             // id값 추출
        QString name = clientModel->data(idx.siblingAtColumn(1)).toString();    // name 추출
        QString number = clientModel->data(idx.siblingAtColumn(2)).toString();  // number 추출
        QStringList strings;                                                    // QStringList 변수선언
        strings << QString::number(id) << name << number;                       // QStringList에 QString 형으로 저장

        QList<QStandardItem *> items;                                           // 사용자정의 item을 넣기위한 QList 선언
        for (int i = 0; i < 3; ++i) {                                           // 사용자정의 컬럼수 iterator
            items.append(new QStandardItem(strings.at(i)));                     // QList에 append
        }

        sclientModel->appendRow(items);                                         // 사용자정의 모델에 items append
        sclientModel->setHeaderData(0, Qt::Horizontal, tr("ID"));               // 0번째 컬럼헤더이름 ID로 정의
        sclientModel->setHeaderData(1, Qt::Horizontal, tr("Name"));             // 1번째 컬럼헤더이름 Name로 정의
        sclientModel->setHeaderData(2, Qt::Horizontal, tr("Phone Number"));     // 2번째 컬럼헤더이름 Phone Number로 정의
        ui->searchTreeView->setModel(sclientModel);                             // searchTreeview의 타겟모델 sclientModel로 정의
        for(int i=0;i<sclientModel->columnCount();i++){                         // sclientModel의 컬럼수만큼 iterator
            ui->searchTreeView->resizeColumnToContents(i);                      // column의 size 재조정
        }
    }
}

void ClientManagerForm::on_modifyPushButton_clicked()                           // Modify버튼 - Item수정
{
    QModelIndex model = ui->clienttreeView->currentIndex();                     // treeview에 선택한 model 저장
    QSqlDatabase db = QSqlDatabase::database("clientitemConnection");           // DB접속
    if(db.isOpen()){                                                            // DB접속이 된다면
        QString name, number, address;                                          // 이름,전화번호,주소 QString형 변수 생성
        name = ui->nameLineEdit->text();                                        // nameLineEdit의 text를 name변수에 저장
        number = ui->phoneNumberLineEdit->text();                               // phoneNumberLineEdit의 text를 number변수에 저장
        address = ui->addressLineEdit->text();                                  // addressLineEdit의 text를 address변수에 저장

        clientModel->setData(model.siblingAtColumn(1), name);                   // name을 선택된 model의 1번째컬럼에 저장
        clientModel->setData(model.siblingAtColumn(2), number);                 // number을 선택된 model의 2번째컬럼에 저장
        clientModel->setData(model.siblingAtColumn(3), address);                // address을 선택된 model의 3번째컬럼에 저장
        clientModel->submit();                                                  // clientModel commit
    }
}

void ClientManagerForm::on_addPushButton_clicked()                              // Add버튼 - Item추가
{
    QString name, number, address;                                              // 이름,전화번호,주소 QString형 변수 생성
    int id = makeId();                                                          // id생성후 id변수에 저장
    name = ui->nameLineEdit->text();                                            // nameLineEdit에 text를 name변수에 저장
    number = ui->phoneNumberLineEdit->text();                                   // phoneNumberLineEdit의 text를 number변수에 저장
    address = ui->addressLineEdit->text();                                      // addressLineEdit의 text를 address변수에 저장
    QSqlDatabase db = QSqlDatabase::database("clientitemConnection");           // DB 접속
    if(db.isOpen() && name.length()) {                                          // DB 접속 및 name이 존재한다면
        QSqlQuery query(clientModel->database());                               // query문 타겟은 clientModel의 DB로 설정
        query.prepare("INSERT INTO clientitem VALUES (?, ?, ?, ?)");            // clientItem에 값 저장
        query.bindValue(0, id);                                                 // 0번째 인자에 id 저장
        query.bindValue(1, name);                                               // 1번째 인자에 name 저장
        query.bindValue(2, number);                                             // 2번째 인자에 number 저장
        query.bindValue(3, address);                                            // 3번째 인자에 address 저장
        query.exec();                                                           // query 시작
        clientModel->select();                                                  // clientModel 업데이트

        emit clientAdded(id, name);                                             // 추가되면 ChatServer에 추가되도록 설정
    }
}

void ClientManagerForm::on_deletePushButton_clicked()                           // Delete버튼 - Item삭제
{
    removeItem();                                                               // removeItem() 멤버함수 호출
}

void ClientManagerForm::clientIdListData(int index)                             // ClientComoboBox의 Index에 따른 ClientList를 가져오기 위함
{
    Q_UNUSED(index);                                                            // index 사용안함
    QString Idstr;                                                              // Id와 이름을 QString형으로 같이 저장하기위한 변수
    QList<QString> IdList;                                                      // Idstr인 QString을 저장하기위한 QList 변수

    for(int i = 0; i < clientModel->rowCount(); i++) {                          // clientModel의 item 개수만큼 iterator
        QString id = clientModel->data(clientModel->index(i, 0)).toString();    // id추출
        QString name = clientModel->data(clientModel->index(i, 1)).toString();  // name 추출
        Idstr = QString("%1, %2").arg(id).arg(name);                            // "ID, 이름" 형태의 QString형 변수 저장
        IdList.append(Idstr);                                                   // 추출한 String값을 List에 저장
    }

    emit clientDataListSent(IdList);                                            // ClientList를 OrderForm에 전달하기위한 시그널
}

void ClientManagerForm::clientNameListData(QString name)                        // ClientComboBox의 name설정시 이름에따른 ClientList 가져오기 위함
{
    QSqlDatabase db = QSqlDatabase::database("clientitemConnection");           // DB접속
    QSqlTableModel query(this,db);                                              // query문 사용할 Model 설정
    query.QSqlQueryModel::setQuery(QString("select * "
                                           "from clientitem "
                                           "where c_name "
                                           "like '%%1%' "
                                           "order by c_id").arg(name),db);

    for(int i = 0; i < query.rowCount(); i++) {                                 // DB모델의 item개수만큼 iterator
        int id = query.data(clientModel->index(i, 0)).toInt();                  // id 추출
        QString name = query.data(clientModel->index(i, 1)).toString();         // name 추출
        QString number = query.data(clientModel->index(i, 2)).toString();       // number 추출
        QString address = query.data(clientModel->index(i, 3)).toString();      // address 추출
        emit clientFindDataSent(id,name,number,address);                        // 추출한 4개의 변수를 signal로 전달
    }
}


void ClientManagerForm::on_clienttreeView_clicked(const QModelIndex &index)             // TreeView의고객창 선택버튼 - 고객메뉴로 선택됨
{
    Q_UNUSED(index);                                                                    // column인자 사용안함
    QModelIndex model = ui->clienttreeView->currentIndex();                             // 선택된 model의 index 저장
    ui->idLineEdit->setText(model.sibling(model.row(),0).data().toString());            // idLineEdit에 item의 text(0)값을 저장
    ui->nameLineEdit->setText(model.sibling(model.row(),1).data().toString());          // nameLineEdit에 item의 text(1)값을 저장
    ui->phoneNumberLineEdit->setText(model.sibling(model.row(),2).data().toString());   // phoneNumberLineEdit item의 text(2)값을 저장
    ui->addressLineEdit->setText(model.sibling(model.row(),3).data().toString());       // addressLineEdit에 item의 text(3)값을 저장
    ui->toolBox->setCurrentIndex(0);                                                    // toolbox화면상태 0번으로 변경
}


void ClientManagerForm::on_searchTreeView_doubleClicked(const QModelIndex &index)       // TreeView의검색창 선택버튼 - 검색메뉴로 선택됨
{
    Q_UNUSED(index);                                                                    // column인자 사용안함
    QModelIndex model = ui->searchTreeView->currentIndex();
    ui->idLineEdit->setText(model.sibling(model.row(),0).data().toString());            // idLineEdit에 item의 text(0)값을 저장
    ui->nameLineEdit->setText(model.sibling(model.row(),1).data().toString());          // nameLineEdit에 item의 text(1)값을 저장
    ui->phoneNumberLineEdit->setText(model.sibling(model.row(),2).data().toString());   // phoneNumberLineEdit item의 text(2)값을 저장
    ui->addressLineEdit->setText(model.sibling(model.row(),3).data().toString());       // addressLineEdit에 item의 text(3)값을 저장
    ui->toolBox->setCurrentIndex(0);                                                    // toolbox화면상태 0번으로 변경
}

