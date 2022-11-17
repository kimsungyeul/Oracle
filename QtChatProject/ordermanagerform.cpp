#include "ordermanagerform.h"
#include "ui_ordermanagerform.h"

#include <QFile>
#include <QMenu>
#include <QDateTime>
#include <QMessageBox>

OrderManagerForm::OrderManagerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;                                                               // TreeWidget과 ToolBox간 거리 조절용
    sizes << 320 << 600;                                                            // 사이즈 지정
    ui->splitter->setSizes(sizes);                                                  // 사이즈 세팅

    QAction* removeAction = new QAction(tr("&Remove"));                             // Remove Action 생성
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));                 // Remove Action 동작시 removeItem슬롯 실행

    menu = new QMenu;                                                               // Action을 동작하기위한 메뉴생성 멤버변수
    menu->addAction(removeAction);                                                  // 액션 추가
    ui->ordertreeView->setContextMenuPolicy(Qt::CustomContextMenu);                 // 위젯의 ContextMenu의 속성을 표시하기 위함
    connect(ui->ordertreeView, SIGNAL(customContextMenuRequested(QPoint)),          // ordertreeView내부에서 우클릭동작시 ContextMenu를 실행하기 위함
            this, SLOT(showContextMenu(QPoint)));
}

void OrderManagerForm::loadData()                                                   // 프로그램 실행시 clientlist값을 불러오기 위함
{
     // QSQLITE를 DB로 사용 및 orderitemConnection으로 DB접속선언
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "orderitemConnection");
    db.setDatabaseName("orderitem.db");                                             // orderitem.db를 db이름으로 설정
    if (db.open()) {                                                                // db가 열리면
        QSqlQuery query(db);                                                        // query사용타켓을 db로 설정
        query.exec("CREATE TABLE IF NOT EXISTS orderitem("
                   "o_id INTEGER Primary Key, "
                   "o_c_id INTEGER,"
                   "o_c_name VARCHAR(30) NOT NULL, "
                   "o_p_id INTEGER,"
                   "o_p_name VARCHAR(30) NOT NULL, "
                   "o_amount INTEGER, "
                   "o_totprice INTEGER, "
                   "o_date date NOT NULL);");

        orderModel = new QSqlTableModel(this, db);                                  // base모델을 db로 지정
        orderModel->setTable("orderitem");                                          // 테이블명을 orderitem으로 지정
        orderModel->select();                                                       // model의 select문으로 전체검색
        orderModel->setHeaderData(0, Qt::Horizontal, tr("OID"));                    // 0번째 컬럼헤더이름 OID로 정의
        orderModel->setHeaderData(1, Qt::Horizontal, tr("CID"));                    // 1번째 컬럼헤더이름 CID로 정의
        orderModel->setHeaderData(2, Qt::Horizontal, tr("CName"));                  // 2째 컬럼헤더이름 CName로 정의
        orderModel->setHeaderData(3, Qt::Horizontal, tr("PID"));                    // 3번째 컬럼헤더이름 PID로 정의
        orderModel->setHeaderData(4, Qt::Horizontal, tr("PName"));                  // 4번째 컬럼헤더이름 Pname로 정의
        orderModel->setHeaderData(5, Qt::Horizontal, tr("Amount"));                 // 5번째 컬럼헤더이름 Amount로 정의
        orderModel->setHeaderData(6, Qt::Horizontal, tr("Total Price"));            // 6번째 컬럼헤더이름 Total Price로 정의
        orderModel->setHeaderData(7, Qt::Horizontal, tr("Date"));                   // 7번째 컬럼헤더이름 Date로 정의

        ui->ordertreeView->setModel(orderModel);                                    // ordertreeView의 타겟모델을 orderModel로 지정
        ui->ordertreeView->setRootIsDecorated(false);                               // 컬럼의 첫항목 공백 제거

        for(int i=0;i<orderModel->columnCount();i++){                               // orderModel의 컬럼수만큼 iterator
            ui->ordertreeView->resizeColumnToContents(i);                           // 컬럼별 자동 줄맞춤
        }
    }
}

OrderManagerForm::~OrderManagerForm()                                               // OrderManagerForm 소멸자
{
    delete ui;
    QSqlDatabase db = QSqlDatabase::database("orderitemConnection");                // order DB접속
    if(db.isOpen()) {                                                               // db가 열린다면
        orderModel->submitAll();                                                    // db값 commitAll
        db.close();                                                                 // db 닫기
    }
}

int OrderManagerForm::makeOId()                                                     // Order Id를 부여하기위한 멤버함수
{
    if(orderModel->rowCount() == 0) {                                               // orderModel의 데이터가 없다면
        return 5000;                                                                // oid를 5000번부터 부여
    } else {
        int lastNum = orderModel->rowCount();                                       // 마지막 아에팀 개수
        auto oid = orderModel->data(orderModel->index(lastNum-1, 0)).toInt();       // orderModel의 마지막값을 id로 가져와
        return ++oid;                                                               // +1 하여 아이디 부여
    }
}

void OrderManagerForm::removeItem()                                                 // treeView의 항목 제거용
{
    QModelIndex model = ui->ordertreeView->currentIndex();                          // 선택된 항목을 저장
    QSqlDatabase db = QSqlDatabase::database("orderitemConnection");                // order DB접속
    if(db.isOpen() && model.isValid()) {                                            // DB가 열리고 model이 유효하면
        orderModel->removeRow(model.row());                                         // 선택된 model의 row값 제거(item제거)
        orderModel->select();                                                       // 제거후 전체표시(업데이트)
    }
}

void OrderManagerForm::showContextMenu(const QPoint &pos)                           // treeView내에서 우클릭시 action을 사용하기 위함
{
    QPoint globalPos = ui->ordertreeView->mapToGlobal(pos);                         // 우클릭된 모니터의 좌표값 불러오기
    if(ui->ordertreeView->indexAt(pos).isValid())                                   // 선택된 좌표값에 index가 있다면
        menu->exec(globalPos);                                                      // menu Action 실행
}

void OrderManagerForm::on_clientcomboBox_currentIndexChanged(int index)             // clientcomboBox의 index변경시 실행되는 슬롯
{
    ui->clientInfocomboBox->clear();                                                // 현재 clientInfocomboBox 데이터 클리어

    if (index == 0) {                                                               // index가 0이라면
        ui->clientInfocomboBox->setEditable(false);                                 // clientInfocomboBox 수정불가
        ui->clienttreeWidget->clear();                                              // clienttreeWidget 클리어
        ui->clientInfocomboBox->clear();                                            // clientInfocomboBox 클리어
    }
    else if (index == 1) {                                                          // index가 1이라면
        ui->clientInfocomboBox->setEditable(false);                                 // clientInfocomboBox 수정불가
        emit clientDataSent(index);                                                 // index값을 clientmanager로 보냄
    }
    else if (index == 2) {                                                          // index가 2이라면
        ui->clientInfocomboBox->setEditable(true);                                  // clientInfocomboBox 수정가능
        ui->clientInfocomboBox->setMaxVisibleItems(5);                              // clientInfocomboBox 목록 보이는 최대갯수 5개
    }
}

void OrderManagerForm::on_clientInfocomboBox_textActivated(const QString &arg1)     // clientInfocomboBox의 text가 활성화시 실행되는 슬롯
{
    ui->clienttreeWidget->clear();                                                  // 현재 clienttreeWidget 데이터 클리어

    if(ui->clientcomboBox->currentIndex() == 1){                                    // clientcomboBox의 선택index가 1이라면
        QList<QString> row = arg1.split(", ");                                      // ", " 기준으로 잘라서 List에 담는다
        emit clientDataSent(row[1]);                                                // QList의 1번째인자(name)을 clientManager로 보냄
    }
    else if(ui->clientcomboBox->currentIndex() == 2){                               // clientcomboBox의 선택index가 2이라면
        emit clientDataSent(arg1);                                                  // text에 적혀진 이름을 clientManager로 보냄
    }
}

void OrderManagerForm::clientFindDataRecv(int id,QString name,                      // ClientItem에서 검색된 값을 받기위한 슬롯
                                          QString number,QString address)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->clienttreeWidget);              // clienttreeWidget을 부모로하는 item 위젯 생성
    if(id != NULL) {                                                                // id값이 null이 아니면
        item->setText(0,QString::number(id));                                       // item의 0번을 id로 저장
        item->setText(1,name);                                                      // item의 1번을 name로 저장
        item->setText(2,number);                                                    // item의 2번을 number로 저장
        item->setText(3,address);                                                   // item의 3번을 address로 저장
    }
    for(int i = 0; i < ui->clienttreeWidget->columnCount(); i++){                   // treeWidget의 컬럼개수만큼 iterator
        ui->clienttreeWidget->resizeColumnToContents(i);                            // 컬럼의 간격 재정의
    }
}



void OrderManagerForm::clientDataListRecv(QList<QString> IdList)                    // Client DB에서 뽑은 cid, cname의 QList형으로 받기위한 슬롯
{
    ui->clientInfocomboBox->addItems(IdList);                                       // clientInfocomboBox에 항목추가
}

void OrderManagerForm::on_clienttreeWidget_itemDoubleClicked(QTreeWidgetItem *item, // clienttreeWidget의 항목이 클릭시 실행되는 슬롯
                                                             int column)
{
    Q_UNUSED(column);                                                               // column 사용안함

    QString NameIdstr;                                                              // NameIdstr변수선언
    int cid = item->text(0).toInt();                                                // item의 text(0)을 cid에 저장
    QString name = item->text(1);                                                   // item의 text(1)을 name에 저장
    NameIdstr = QString("%1(%2)").arg(name).arg(cid);                               // 고객이름(고객번호) String형태로 저장

    if(item != nullptr) {                                                           // TreeWidgetItem이 비어있지않다면
        ui->nameLineEdit->setText(NameIdstr);                                       // nameLineEdit에 text저장
    }
}

void OrderManagerForm::on_productcomboBox_currentIndexChanged(int index)            // productcomboBox의 index변경시 실행되는 슬롯
{
    ui->productInfocomboBox->clear();                                               // productInfocomboBox 클리어

    if (index == 0) {                                                               // index가 0이라면
        ui->productInfocomboBox->setEditable(false);                                // productInfocomboBox 수정불가
        ui->producttreeWidget->clear();                                             // producttreeWidget 클리어
        ui->productInfocomboBox->clear();                                           // productInfocomboBox 클리어
    }
    else if (index == 1) {                                                          // index가 1이라면
        ui->productInfocomboBox->setEditable(false);                                // productInfocomboBox 수정불가
        emit productDataSent(index);                                                // index값을 productmanager로 보냄
    }
    else if (index == 2) {                                                          // index가 2이라면
        ui->productInfocomboBox->setEditable(true);                                 // productInfocomboBox 수정가능
        ui->productInfocomboBox->setMaxVisibleItems(5);                             // productInfocomboBox 목록 보이는 최대갯수 5개
    }
}

void OrderManagerForm::on_productInfocomboBox_textActivated(const QString &arg1)    // productInfocomboBox의 text가 활성화시 실행되는 슬롯
{
    ui->producttreeWidget->clear();                                                 // 현재 producttreeWidget 데이터 클리어

    if(ui->productcomboBox->currentIndex() == 1){                                   // productcomboBox의 선택index가 1이라면
        QList<QString> row = arg1.split(", ");                                      // ", " 기준으로 잘라서 List에 담는다
        emit productDataSent(row[1]);                                               // QList의 1번째인자(name)을 productManager로 보냄
    }
    else if(ui->productcomboBox->currentIndex() == 2){                              // productcomboBox의 선택index가 2이라면
        emit productDataSent(arg1);                                                 // text에 적혀진 이름을 productManager로 보냄
    }
}

void OrderManagerForm::productFindDataRecv(int pid,QString pname,                   // productList에서 검색된 값을 받기위한 슬롯
                                           int price ,int stock)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->producttreeWidget);             // producttreeWidget을 부모로하는 item 객체생성
    if(pid != NULL) {                                                               // pid값이 NULL이 아니면
        item->setText(0,QString::number(pid));                                      // item의 0번을 pid로 저장
        item->setText(1,pname);                                                     // item의 1번을 id로 저장
        item->setText(2,QString::number(price));                                    // item의 2번을 price로 저장
        item->setText(3,QString::number(stock));                                    // item의 3번을 stock로 저장
    }
    for(int i = 0; i < ui->producttreeWidget->columnCount(); i++){                  // producttreeWidget의 컬럼수만큼 iterator
        ui->producttreeWidget->resizeColumnToContents(i);                           // 각 컬럼별 사이즈 재조정
    }
}

void OrderManagerForm::productDataListRecv(QList<QString> PIdList)                  // product DB에서 뽑은 pid, pname의 QList형으로 받기위한 슬롯
{
    ui->productInfocomboBox->addItems(PIdList);                                     // productInfocomboBox에 item항목추가
}

void OrderManagerForm::on_producttreeWidget_itemDoubleClicked(QTreeWidgetItem *item,// producttreeWidget의 항목이 클릭시 실행되는 슬롯
                                                              int column)
{
    Q_UNUSED(column);                                                               // column 사용안함

    QString NameIdstr;                                                              // NameIdstr변수선언
    int pid = item->text(0).toInt();                                                // item의 text(0)을 pid에 저장
    QString pname = item->text(1);                                                  // item의 text(1)을 pname에 저장
    int stock = item->text(3).toInt();                                              // item의 text(3)을 stock에 저장
    NameIdstr = QString("%1(%2)").arg(pname).arg(pid);                              // 제품이름(제품번호) String형태로 저장

    if(item != nullptr) {                                                           // TreeWidgetItem이 비어있지않다면
        ui->pnameLineEdit->setText(NameIdstr);                                      // pnameLineEdit text저장
        ui->priceLineEdit->setText(item->text(2));                                  // priceLineEdit item->text(2)저장
        ui->stockLineEdit->setText(item->text(3));                                  // stockLineEdit item->text(3)저장
        ui->amountspinBox->setMaximum(stock);                                       // amountspinBox stock저장
    }
}

void OrderManagerForm::on_addpushButton_clicked()                                   // Add버튼 - Item추가
{
    QString citemname = ui->nameLineEdit->text();                                   // nameLienEdit의 text값을 clientitem 이름변수 저장
    QString pitemname = ui->pnameLineEdit->text();                                  // pnameLienEdit의 text값을 productitem 이름변수 저장
    // Order Number
    if((citemname.isEmpty() != true) && (pitemname.isEmpty() != true)) {            // client product 의 name변수가 비어있지 않다면
        int oid = makeOId();                                                        // oid번호 생성
        QList<QString> clientrow = ui->nameLineEdit->text().split("(");             // nameLineEdit을 "("구분자 기준으로 분할
        QList<QString> productrow = ui->pnameLineEdit->text().split("(");           // pnameLineEdit을 "("구분자 기준으로 분할
        clientrow[1] = clientrow[1].remove(QChar(')'), Qt::CaseInsensitive);        // 분할된 name의 1번인자의 ')' 기호 삭제
        productrow[1] = productrow[1].remove(QChar(')'), Qt::CaseInsensitive);      // 분할된 pname의 1번인자의 ')' 기호 삭제
        // Client Name, Item Name
        int cid = clientrow[1].toInt();                                             // 분할된 cid의 1번인자를뽑아 cid에 저장
        QString cname = clientrow[0];                                               // 분할된 name의 0번인자를뽑아 cname에 저장
        int pid = productrow[1].toInt();                                            // 분할된 pid의 1번인자를뽑아 pid에 저장
        QString pname = productrow[0];                                              // 분할된 pname의 0번인자를뽑아 pname에 저장
        int amount = ui->amountspinBox->text().toInt();                             // amountspinBox의 값을 amount변수에 저장
        int totprice = ui->totalpriceLineEdit->text().toInt();                      // totalpriceLineEdit의 값을 totprice변수에 저장
        // Date
        QString time_format = "yyyy-MM-dd HH:mm:ss";                                // Date의 형식 지정
        QString date = QDateTime::currentDateTime().toString(time_format);          // 최근시간을 Date형식에 맞게 가져와 date변수에 저장

        QSqlDatabase db = QSqlDatabase::database("orderitemConnection");            // orderitem DB연결
        if(db.isOpen()) {                                                           // DB가 열린다면
            QSqlQuery query(orderModel->database());                                // orderModel의 DB를타깃으로한 query선언문 생성
            query.prepare("INSERT INTO orderitem VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
            query.bindValue(0, oid);
            query.bindValue(1, cid);
            query.bindValue(2, cname);
            query.bindValue(3, pid);
            query.bindValue(4, pname);
            query.bindValue(5, amount);
            query.bindValue(6, totprice);
            query.bindValue(7, date);
            query.exec();
            orderModel->select();
            for(int i=0;i<orderModel->columnCount();i++){                           // orderModel의 컬럼수만큼 iterator
                ui->ordertreeView->resizeColumnToContents(i);                       // 컬럼간 사이즈 조절
            }

            emit stockUpdate(pid,amount);                                           // product의 stock 업데이트를위한 시그널
        }
    } else {
        QMessageBox::critical(this, tr("Invalid Order"),                          // client or product name칸에 하나라도 없다면 경고메시지 발생
                              tr("PlZ Client or Product input"));
    }
}


void OrderManagerForm::on_amountspinBox_valueChanged(int arg1)                      // amountspinBox의 값이 바뀌면
{
    ui->totalpriceLineEdit->setText(QString::number(arg1 * ui->priceLineEdit->      // totalpriceLineEdit의 값을 priceLineEdit에 입력된 값으로 설정
                                                    text().toInt()));
}

void OrderManagerForm::getProductIdDataRecv(int pid, QString pname,                 // stock의 값을 최신화하기위한 슬롯
                                            QString price ,QString stock)
{
    ui->producttreeWidget->clear();                                                 // producttreewidget 클리어

    QTreeWidgetItem *item = new QTreeWidgetItem(ui->producttreeWidget);             // producttreewidget을 부모로하는 item 선언
    if(pid != NULL) {                                                               // pid가 NULL값이 아니면
        item->setText(0,QString::number(pid));                                      // item의 0번째인자에 pid값 설정
        item->setText(1,pname);                                                     // item의 1번째인자에 pname값 설정
        item->setText(2,price);                                                     // item의 2번째인자에 price값 설정
        item->setText(3,stock);                                                     // item의 3번째인자에 stock값 설정
    }
    ui->producttreeWidget->addTopLevelItem(item);                                   // producttreeWidget의 Item등록

    QString maxstock = item->text(3);                                               // item 3번째인자에서 maxstock값 추출
    ui->priceLineEdit->setText(item->text(2));                                      // priceLineEdit의값을 item의 2번인자에 등록
    ui->amountspinBox->setMaximum(maxstock.toInt());                                // 추출한 maxstock값을 amountspinBox의 maximum값으로 설정
    ui->stockLineEdit->setText(maxstock);                                           // stockLineEdit에 maxstock값 설정
}

void OrderManagerForm::on_deletepushButton_clicked()                                // Delete버튼 - Item삭제
{
    removeItem();                                                                   // item삭제
}


void OrderManagerForm::on_modifypushButton_clicked()                                // Modify버튼 - Item수정
{
    QModelIndex model = ui->ordertreeView->currentIndex();                          // ordertreeview의 선택된 index값을 modelidex객체로 지정

    if(model.isValid() != NULL) {                                                   // model이 NULL값이 아니면
        int befamount, afamount, totalPrice;                                        // 이전amount,이후amount,전체금액변수 선언

        int pid = model.sibling(model.row(),3).data().toInt();;                     // 선택된 model의 3번째데이터를 추출해 pid값에 저장

        befamount = model.sibling(model.row(),5).data().toInt();;                   // 선택된 model의 5번째데이터를 추출해 befamount값에 저장
        afamount = ui->amountspinBox->value();                                      // amountspinBox데이터를 추출해 afamount값에 저장
        totalPrice = ui->totalpriceLineEdit->text().toInt();                        // totalpriceLineEdit의 text데이터를 추출해 totalPrice값에 저장

        QSqlDatabase db = QSqlDatabase::database("orderitemConnection");            // order DB에 연결
        if(db.isOpen()){                                                            // db가 연결된다면
            orderModel->setData(model.siblingAtColumn(5), afamount);                // orderModel의 5번째인자에 afamount값을 저장(업데이트)
            orderModel->setData(model.siblingAtColumn(6), totalPrice);              // orderModel의 6번째인자에 totalPrice값을 저장(업데이트)
            orderModel->submit();                                                   // orderModel commit
        }

        emit stockUpdate(pid,afamount-befamount);                                   // product의 stock없데이트를 위한 시그널
    }
}

void OrderManagerForm::udstockRecv(int stock)                                       // update된 stock값을 받기위한 슬롯함수
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->producttreeWidget->             // 선택된 productwidget을 부모로하는 item객체생성
                                                currentItem());
    if(item != NULL) {                                                              // item이 NULL이 아니면
        item->setText(3,QString::number(stock));                                    // item의 3번째인자에 stock값 설정
        ui->producttreeWidget->update();                                            // producttreeWidget창 업데이트
    }
}

void OrderManagerForm::on_ordertreeView_clicked(const QModelIndex &index)           // ordertreeView 클릭시
{
    int productID = index.sibling(index.row(),3).data().toInt();                    // 선택된 index의 3번째인자인 productID값 추출

    emit getProductItemSent(productID);                                             // productID를 Productitem으로 보내기위한 시그널

    ui->nameLineEdit->setText(index.sibling(index.row(),2).data().toString());      // nameLineEdit에 선택된 index의 2번째인자값을 설정
    ui->pnameLineEdit->setText(index.sibling(index.row(),4).data().toString());     // pnameLineEdit에 선택된 index의 4번째인자값을 설정
    ui->totalpriceLineEdit->setText(index.sibling(index.row(),6).data().toString());// totalpriceLineEdit에 선택된 index의 6번째인자값을 설정
}
