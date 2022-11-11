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
    ui->clienttreeView->setContextMenuPolicy(Qt::CustomContextMenu);          // 위젯의 ContextMenu의 속성을 표시하기 위함
    connect(ui->clienttreeView, SIGNAL(customContextMenuRequested(QPoint)),   // clienttreeWidget내부에서 우클릭동작시 ContextMenu를 실행하기 위함
            this, SLOT(showContextMenu(QPoint)));
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),                        // searchLineEdit의 returnPressed동작시 searchPushButton슬롯 실행
            this, SLOT(on_searchPushButton_clicked()));

    sclientModel = new QStandardItemModel(0,3);
    sclientModel->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    sclientModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));
    sclientModel->setHeaderData(2, Qt::Horizontal, QObject::tr("PhonNumber"));
    ui->searchTreeView->setModel(sclientModel);
    ui->searchTreeView->setRootIsDecorated(false);
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

        ui->clienttreeView->setModel(clientModel);
        ui->clienttreeView->setRootIsDecorated(false);
    }

    for(int i = 0; i < clientModel->rowCount(); i++) {
        int id = clientModel->data(clientModel->index(i, 0)).toInt();
        QString name = clientModel->data(clientModel->index(i, 1)).toString();
        emit clientAdded(id, name);
    }
}

ClientManagerForm::~ClientManagerForm()                                         // ClientManagerForm 소멸자
{
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
        clientModel->removeRow(model.row());
        clientModel->select();
    }
}

void ClientManagerForm::showContextMenu(const QPoint &pos)                      // treewidget내에서 우클릭시 action을 사용하기 위함
{
    QPoint globalPos = ui->clienttreeView->mapToGlobal(pos);                  // 우클릭된 모니터의 좌표값 불러오기
    if(ui->clienttreeView->indexAt(pos).isValid())
        menu->exec(globalPos);
}

void ClientManagerForm::on_searchPushButton_clicked()                           // Search버튼 - Item검색
{
    sclientModel->clear();
    int i = ui->searchComboBox->currentIndex();                       // 검색할 id,이름,전화번호, 타입을 설정
    auto flag = (i)? Qt::MatchFixedString|Qt::MatchContains
                   : Qt::MatchCaseSensitive;
    QModelIndexList indexes = clientModel->match(clientModel->index(0, i), Qt::EditRole,
                                                 ui->searchLineEdit->text(),
                                                 -1, Qt::MatchFlags(flag));
    foreach(auto idx, indexes) {
        int id = clientModel->data(idx.siblingAtColumn(0)).toInt();
        QString name = clientModel->data(idx.siblingAtColumn(1)).toString();
        QString number = clientModel->data(idx.siblingAtColumn(2)).toString();
        QStringList strings;
        strings << QString::number(id) << name << number;

        QList<QStandardItem *> items;
        for (int i = 0; i < 3; ++i) {
            items.append(new QStandardItem(strings.at(i)));
        }

        sclientModel->appendRow(items);
        sclientModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
        sclientModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
        sclientModel->setHeaderData(2, Qt::Horizontal, tr("Phone Number"));
        ui->searchTreeView->setModel(sclientModel);
        for(int i=0;i<clientModel->columnCount();i++){
            ui->searchTreeView->resizeColumnToContents(i);
        }
    }
}

void ClientManagerForm::on_modifyPushButton_clicked()                           // Modify버튼 - Item수정
{
    QModelIndex model = ui->clienttreeView->currentIndex();
    QSqlDatabase db = QSqlDatabase::database("clientitemConnection");
    if(db.isOpen()){
        int key = model.sibling(model.row(),0).data().toInt();                                        // 0번째항목에서 key값추출
        QString name, number, address;                                          // 이름,전화번호,주소 QString형 변수 생성
        name = ui->nameLineEdit->text();                                        // nameLineEdit의 text를 name변수에 저장
        number = ui->phoneNumberLineEdit->text();                               // phoneNumberLineEdit의 text를 number변수에 저장
        address = ui->addressLineEdit->text();                                  // addressLineEdit의 text를 address변수에 저장

        clientModel->setData(model.siblingAtColumn(1), name);
        clientModel->setData(model.siblingAtColumn(2), number);
        clientModel->setData(model.siblingAtColumn(3), address);
        clientModel->submit();
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
        for(int i=0;i<clientModel->columnCount();i++){
            ui->clienttreeView->resizeColumnToContents(i);
        }

        emit clientAdded(id, name);
    }
}

void ClientManagerForm::on_deletePushButton_clicked()                           // Delete버튼 - Item삭제
{
    removeItem();                                                               // removeItem() 멤버함수 호출
}

void ClientManagerForm::clientIdListData(int index)                             // ClientComoboBox의 Index에 따른 ClientList를 가져오기 위함
{
    QString Idstr;                                                              // Id와 이름을 QString형으로 같이 저장하기위한 변수
    QList<QString> IdList;                                                      // Idstr인 QString을 저장하기위한 QList 변수

    for(int i = 0; i < clientModel->rowCount(); i++) {
        QString id = clientModel->data(clientModel->index(i, 0)).toString();
        QString name = clientModel->data(clientModel->index(i, 1)).toString();
        Idstr = QString("%1, %2").arg(id).arg(name);                    // "ID, 이름" 형태의 QString형 변수 저장
        IdList.append(Idstr);
    }

    emit clientDataListSent(IdList);                                            // ClientList를 OrderForm에 전달하기위한 시그널
}

void ClientManagerForm::clientNameListData(QString name)                        // ClientComboBox의 name설정시 이름에따른 ClientList 가져오기 위함
{
    QSqlDatabase db = QSqlDatabase::database("clientitemConnection");
    QSqlTableModel query(this,db);
    query.QSqlQueryModel::setQuery(QString("select * "
                                           "from clientitem "
                                           "where c_name "
                                           "like '%%1%' "
                                           "order by c_id").arg(name),db);

    for(int i = 0; i < query.rowCount(); i++) {
        int id = query.data(clientModel->index(i, 0)).toInt();
        QString name = query.data(clientModel->index(i, 1)).toString();
        QString number = query.data(clientModel->index(i, 2)).toString();
        QString address = query.data(clientModel->index(i, 3)).toString();
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

