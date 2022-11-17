#include "productmanagerform.h"
#include "ui_productmanagerform.h"

#include <QFile>
#include <QMenu>

ProductManagerForm::ProductManagerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProductManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;                                                               // TreeWidget과 ToolBox간 거리 조절용
    sizes << 320 << 600;                                                            // 사이즈 지정
    ui->splitter->setSizes(sizes);                                                  // 사이즈 세팅

    QAction* removeAction = new QAction(tr("&Remove"));                             // Remove Action 생성
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));                 // Remove Action 동작시 removeItem슬롯 실행

    menu = new QMenu;                                                               // Action을 동작하기위한 메뉴생성 멤버변수
    menu->addAction(removeAction);                                                  // 액션 추가
    ui->producttreeView->setContextMenuPolicy(Qt::CustomContextMenu);               // 위젯의 ContextMenu의 속성을 표시하기 위함
    connect(ui->producttreeView, SIGNAL(customContextMenuRequested(QPoint)),        // producttreeView내부에서 우클릭동작시 ContextMenu를 실행하기 위함
            this, SLOT(showContextMenu(QPoint)));
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),                            // searchLineEdit의 returnPressed동작시 searchPushButton슬롯 실행
            this, SLOT(on_searchPushButton_clicked()));

    sproductModel = new QStandardItemModel(0,4);                                    // 사용자정의모델 컬럼 크기 지정
    sproductModel->setHeaderData(0, Qt::Horizontal, QObject::tr("PID"));            // 0번째 컬럼헤더이름 PID 로 정의
    sproductModel->setHeaderData(1, Qt::Horizontal, QObject::tr("PName"));          // 1번째 컬럼헤더이름 PName 로 정의
    sproductModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Price"));          // 2번째 컬럼헤더이름 Price 로 정의
    sproductModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Stock"));          // 3번째 컬럼헤더이름 Stock 로 정의
    ui->searchTreeView->setModel(sproductModel);                                    // searchTreeView에대한 모델을 지정
    ui->searchTreeView->setRootIsDecorated(false);                                  // 첫항목 빈공간 제거
}

void ProductManagerForm::loadData()
{   // QSQLITE를 DB로 사용 및 productitemConnection으로 DB접속선언
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "productitemConnection");
    db.setDatabaseName("productitem.db");                                           // productitem.db를 db이름으로 설정
    if (db.open()) {                                                                // db가 열리면
        QSqlQuery query(db);                                                        // query사용타켓을 db로 설정
        query.exec("CREATE TABLE IF NOT EXISTS productitem("
                   "p_id INTEGER Primary Key, "
                   "p_name VARCHAR(30) NOT NULL, "
                   "p_price INTEGER, "
                   "p_stock INTEGER);");

        productModel = new QSqlTableModel(this, db);                                // base모델을 db로 지정
        productModel->setTable("productitem");                                      // 테이블명을 productitem으로 지정
        productModel->select();                                                     // model의 select문으로 전체검색
        productModel->setHeaderData(0, Qt::Horizontal, tr("PID"));                  // 0번째 컬럼헤더이름 ID로 정의
        productModel->setHeaderData(1, Qt::Horizontal, tr("PName"));                // 1번째 컬럼헤더이름 Name로 정의
        productModel->setHeaderData(2, Qt::Horizontal, tr("Price"));                // 2번째 컬럼헤더이름 Price로 정의
        productModel->setHeaderData(3, Qt::Horizontal, tr("Stock"));                // 3번째 컬럼헤더이름 Stock로 정의

        ui->producttreeView->setModel(productModel);                                // producttreeView의 타겟모델을 productModel로 지정
        ui->producttreeView->setRootIsDecorated(false);                             // 컬럼의 첫항목 공백 제거
    }
}

ProductManagerForm::~ProductManagerForm()                                            // ProductManagerForm 소멸자
{
    delete ui;
    QSqlDatabase db = QSqlDatabase::database("productitemConnection");              // client데이터베이스 접속
    if(db.isOpen()) {                                                               // db가 열린다면
        productModel->submitAll();                                                  // db값 commitAll
        db.close();                                                                 // db 닫기
    }
}

int ProductManagerForm::makePId( )                                                  // Product Id를 부여하기위한 멤버함수
{
    if(productModel->rowCount() == 0) {                                             // productModel의 아이템 데이터가 없다면
        return 8000;                                                                // id를 8000번부터 부여
    } else {
        int lastNum = productModel->rowCount();                                     // 마지막 아이템개수
        auto pid = productModel->data(productModel->index(lastNum-1, 0)).toInt();   // productModel의 마지막값을 id로 가져와
        return ++pid;                                                               // +1 하여 아이디 부여
    }
}

void ProductManagerForm::removeItem()                                               // treewidget의 항목 제거용
{
    QModelIndex model = ui->producttreeView->currentIndex();                        // 선택된 항목을 저장
    QSqlDatabase db = QSqlDatabase::database("productitemConnection");              // product DB접속
    if(db.isOpen() && model.isValid()) {                                            // DB가 열리고 model이 유효하면
        productModel->removeRow(model.row());                                       // 선택된 model의 row값 제거(item제거)
        productModel->select();                                                     // 제거후 전체표시(업데이트)
    }
}

void ProductManagerForm::showContextMenu(const QPoint &pos)                         // treewidget내에서 우클릭시 action을 사용하기 위함
{
    QPoint globalPos = ui->producttreeView->mapToGlobal(pos);                       // 우클릭된 모니터의 좌표값 불러오기
    if(ui->producttreeView->indexAt(pos).isValid())                                 // 선택된 좌표값에 index가 있다면
        menu->exec(globalPos);                                                      // menu Action 실행
}

void ProductManagerForm::on_searchPushButton_clicked()                              // Search버튼 - Item검색
{
    sproductModel->clear();                                                         // searchModel 클리어
    int i = ui->searchComboBox->currentIndex();                                     // 검색할 pid,pname,price, stock을 설정
    auto flag = (i)? Qt::MatchFixedString|Qt::MatchContains                         // flag옵션 설정
                   : Qt::MatchCaseSensitive;
    QModelIndexList indexes = productModel->match(productModel->index(0, i),        // text와 flag값에 적용되는 모델의 인덱스를 찾아 저장
                                                  Qt::EditRole,
                                                  ui->searchLineEdit->text(),
                                                  -1, Qt::MatchFlags(flag));

    foreach(auto idx, indexes) {                                                    // 찾은 idexes를 찾아 iterator
        int pid = productModel->data(idx.siblingAtColumn(0)).toInt();               // pid값 추출
        QString pname = productModel->data(idx.siblingAtColumn(1)).toString();      // pname값 추출
        int price = productModel->data(idx.siblingAtColumn(2)).toInt();             // price 추출
        int stock = productModel->data(idx.siblingAtColumn(3)).toInt();             // stock 추출
        QStringList strings;                                                        // QStringList 변수선언
        strings << QString::number(pid) << pname                                    // QStringList에 QString 형으로 저장
                << QString::number(price) << QString::number(stock);

        QList<QStandardItem *> items;                                               // 사용자정의 item을 넣기위한 QList 선언
        for (int i = 0; i < 4; ++i) {                                               // 사용자정의 컬럼수 iterator
            items.append(new QStandardItem(strings.at(i)));                         // QList에 append
        }

        sproductModel->appendRow(items);                                            // 사용자정의 모델에 items append
        sproductModel->setHeaderData(0, Qt::Horizontal, tr("PID"));                 // 0번째 컬럼헤더이름 PID로 정의
        sproductModel->setHeaderData(1, Qt::Horizontal, tr("PName"));               // 1번째 컬럼헤더이름 PName로 정의
        sproductModel->setHeaderData(2, Qt::Horizontal, tr("Price"));               // 2번째 컬럼헤더이름 Price로 정의
        sproductModel->setHeaderData(3, Qt::Horizontal, tr("Stock"));               // 3번째 컬럼헤더이름 Stock로 정의
        ui->searchTreeView->setModel(sproductModel);                                // searchTreeview의 타겟모델 sproductModel로 정의
        for(int i=0;i<sproductModel->columnCount();i++){                            // sproductModel의 컬럼수만큼 iterator
            ui->searchTreeView->resizeColumnToContents(i);                          // column의 size 재조정
        }
    }
}

void ProductManagerForm::on_modifyPushButton_clicked()                              // Modify버튼 - Item수정
{
    QModelIndex model = ui->producttreeView->currentIndex();                        // treeview에 선택한 model 저장
    QSqlDatabase db = QSqlDatabase::database("productitemConnection");              // DB접속
    if(db.isOpen()){                                                                // DB접속이 된다면
        QString pname, price, stock;                                                // pname,price,stock을 QString형 변수 생성
        pname = ui->pnameLineEdit->text();                                          // nameLineEdit의 text를 name변수에 저장
        price = ui->priceLineEdit->text();                                          // phoneNumberLineEdit의 text를 number변수에 저장
        stock = ui->stockLineEdit->text();                                          // addressLineEdit의 text를 address변수에 저장

        productModel->setData(model.siblingAtColumn(1), pname);                     // pname을 선택된 model의 1번째컬럼에 저장
        productModel->setData(model.siblingAtColumn(2), price);                     // number을 선택된 model의 2번째컬럼에 저장
        productModel->setData(model.siblingAtColumn(3), stock);                     // address을 선택된 model의 3번째컬럼에 저장
        productModel->submit();                                                     // productModel commit
    }
}


void ProductManagerForm::on_addPushButton_clicked()                                 // Add버튼 - Item추가
{
    QString pname, price, stock;                                                    // pname,price,stock을 QString형 변수 생성
    int pid = makePId();                                                            // pid생성후 pid변수에 저장
    pname = ui->pnameLineEdit->text();                                              // pnameLineEdit에 text를 name변수에 저장
    price = ui->priceLineEdit->text();                                              // priceLineEdit의 text를 number변수에 저장
    stock = ui->stockLineEdit->text();                                              // stockLineEdit의 text를 address변수에 저장
    QSqlDatabase db = QSqlDatabase::database("productitemConnection");              // DB 접속
    if(db.isOpen() && pname.length()) {                                             // DB 접속 및 pname이 존재한다면
        QSqlQuery query(productModel->database());                                  // query문 타겟은 clientModel의 DB로 설정
        query.prepare("INSERT INTO productitem VALUES (?, ?, ?, ?)");               // productItem에 값 저장
        query.bindValue(0, pid);                                                    // 0번째 인자에 pid 저장
        query.bindValue(1, pname);                                                  // 1번째 인자에 pname 저장
        query.bindValue(2, price);                                                  // 2번째 인자에 price 저장
        query.bindValue(3, stock);                                                  // 3번째 인자에 stock 저장
        query.exec();                                                               // query 시작
        productModel->select();                                                     // productModel 업데이트
    }
}

void ProductManagerForm::on_deletePushButton_clicked()                              // Delete버튼 - Item삭제
{
    removeItem();                                                                   // removeItem() 멤버함수 호출
}

void ProductManagerForm::productIdListData(int index)                               // ProductComoboBox의 Index에 따른 ProductList를 가져오기 위함
{
    Q_UNUSED(index);                                                                // index 사용안함
    QString PIdstr;                                                                 // PId와 이름을 QString형으로 같이 저장하기위한 변수
    QList<QString> PIdList;                                                         // Idstr인 QString을 저장하기위한 QList 변수

    for(int i = 0; i < productModel->rowCount(); i++) {                             // productModel의 item 개수만큼 iterator
        QString pid = productModel->data(productModel->index(i, 0)).toString();     // pid 추출
        QString pname = productModel->data(productModel->index(i, 1)).toString();   // pname 추출
        PIdstr = QString("%1, %2").arg(pid).arg(pname);                             // "ID, 이름" 형태의 QString형 변수 저장
        PIdList.append(PIdstr);
    }

    emit productDataListSent(PIdList);                                              // ProductList를 OrderForm에 전달하기위한 시그널
}

void ProductManagerForm::productNameListData(QString pname)                         // ProductComoboBox의 pname설정시 이름에따른 ProductList를 가져오기 위함
{
    QSqlDatabase db = QSqlDatabase::database("productitemConnection");              // DB접속
    QSqlTableModel query(this,db);                                                  // query문 사용할 Model 설정
    query.QSqlQueryModel::setQuery(QString("select * "
                                           "from productitem "
                                           "where p_name "
                                           "like '%%1%' "
                                           "order by p_id").arg(pname),db);
    for(int i = 0; i < query.rowCount(); i++) {                                     // DB모델의 item개수만큼 iterator
        int pid = query.data(productModel->index(i, 0)).toInt();                    // pid 추출
        QString pname = query.data(productModel->index(i, 1)).toString();           // pname 추출
        int price = query.data(productModel->index(i, 2)).toInt();                  // price 추출
        int stock = query.data(productModel->index(i, 3)).toInt();                  // stock 추출
        emit productFindDataSent(pid,pname,price,stock);                            // 추출한 4개의 변수를 signal로 전달
    }
}

void ProductManagerForm::productItemRecv(int pid)                                   // ProductId를 받아 ProductDB의 item값을 추출하기위한 슬롯
{
    QSqlDatabase db = QSqlDatabase::database("productitemConnection");              // DB연결
    QSqlTableModel query(this,db);                                                  // query문 사용할 Model 설정
    query.QSqlQueryModel::setQuery(QString("select * "
                                           "from productitem "
                                           "where p_id = '%1'").arg(pid),db);
    QString pname = query.data(query.index(0, 1)).toString();                       // pname 추출
    QString price = query.data(query.index(0, 2)).toString();                       // price 추출
    QString stock = query.data(query.index(0, 3)).toString();                       // stock 추출

    emit productIdDataSent(pid,pname,price,stock);                                  // 추출한 4개의 변수를 signal로 전달
}

void ProductManagerForm::productStockUp(int pid,int amount)                         // OrderManagerForm에서 변경된 Stock의 pid와 amount를 받아 stock을 update하기위한 슬롯
{
    QSqlDatabase db = QSqlDatabase::database("productitemConnection");              // DB연결
    QSqlTableModel query(this,db);                                                  // query문 사용할 Model 설정
    query.QSqlQueryModel::setQuery(QString("select * "
                                           "from productitem "
                                           "where p_id = '%1'").arg(pid),db);
    int updateStock = query.data(query.index(0, 3)).toInt() - amount;               // stock값 update
    query.QSqlQueryModel::setQuery(QString("update productitem "
                                           "set p_stock = '%1'"
                                           "where p_id = '%2'").arg(updateStock).arg(pid),db);
    query.submit();                                                                 // 윗 query문 commit
    productModel->QSqlQueryModel::setQuery(QString("select * "
                                                   "from productitem"),db);
    productModel->submit();                                                         // select로 업데이트된 productModel 업데이트후 commit

    emit udstockSend(updateStock);                                                  // update된 값을 signal로 보냄
}

void ProductManagerForm::on_producttreeView_clicked(const QModelIndex &index)       // TreeView의고객창 선택버튼 - 제품메뉴로 선택됨
{
    Q_UNUSED(index);                                                                // column인자 사용안함
    QModelIndex model = ui->producttreeView->currentIndex();                        // 선택된 model의 index 저장
    ui->idLineEdit->setText(model.sibling(model.row(),0).data().toString());        // idLineEdit에 item의 text(0)값을 저장
    ui->pnameLineEdit->setText(model.sibling(model.row(),1).data().toString());     // pnameLineEdit에 item의 text(1)값을 저장
    ui->priceLineEdit->setText(model.sibling(model.row(),2).data().toString());     // priceLineEdit item의 text(2)값을 저장
    ui->stockLineEdit->setText(model.sibling(model.row(),3).data().toString());     // stockLineEdit에 item의 text(3)값을 저장
    ui->toolBox->setCurrentIndex(0);                                                // toolbox화면상태 0번으로 변경
}


void ProductManagerForm::on_searchTreeView_doubleClicked(const QModelIndex &index)  // TreeView의검색창 선택버튼 - 검색메뉴로 선택됨
{
    Q_UNUSED(index);                                                                // column인자 사용안함
    QModelIndex model = ui->searchTreeView->currentIndex();
    ui->idLineEdit->setText(model.sibling(model.row(),0).data().toString());        // idLineEdit에 item의 text(0)값을 저장
    ui->pnameLineEdit->setText(model.sibling(model.row(),1).data().toString());     // pnameLineEdit에 item의 text(1)값을 저장
    ui->priceLineEdit->setText(model.sibling(model.row(),2).data().toString());     // priceLineEdit item의 text(2)값을 저장
    ui->stockLineEdit->setText(model.sibling(model.row(),3).data().toString());     // stockLineEdit에 item의 text(3)값을 저장
    ui->toolBox->setCurrentIndex(0);                                                // toolbox화면상태 0번으로 변경
}

