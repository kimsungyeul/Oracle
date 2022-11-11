#include "productmanagerform.h"
#include "ui_productmanagerform.h"

#include <QFile>
#include <QMenu>

ProductManagerForm::ProductManagerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProductManagerForm)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 320 << 600;
    ui->splitter->setSizes(sizes);

    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));

    menu = new QMenu;
    menu->addAction(removeAction);
    ui->producttreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->producttreeView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));

    sproductModel = new QStandardItemModel(0,4);
    sproductModel->setHeaderData(0, Qt::Horizontal, QObject::tr("PID"));
    sproductModel->setHeaderData(1, Qt::Horizontal, QObject::tr("PName"));
    sproductModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Price"));
    sproductModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Stock"));
    ui->searchTreeView->setModel(sproductModel);
    ui->searchTreeView->setRootIsDecorated(false);
}

void ProductManagerForm::loadData()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "productitemConnection");
    db.setDatabaseName("productitem.db");
    if (db.open()) {
        QSqlQuery query(db);
        query.exec("CREATE TABLE IF NOT EXISTS productitem("
                   "p_id INTEGER Primary Key, "
                   "p_name VARCHAR(30) NOT NULL, "
                   "p_price INTEGER, "
                   "p_stock INTEGER);");

        productModel = new QSqlTableModel(this, db);
        productModel->setTable("productitem");
        productModel->select();
        productModel->setHeaderData(0, Qt::Horizontal, tr("PID"));
        productModel->setHeaderData(1, Qt::Horizontal, tr("PName"));
        productModel->setHeaderData(2, Qt::Horizontal, tr("Price"));
        productModel->setHeaderData(3, Qt::Horizontal, tr("Stock"));

        ui->producttreeView->setModel(productModel);
        ui->producttreeView->setRootIsDecorated(false);
    }
}

ProductManagerForm::~ProductManagerForm()
{
    delete ui;
    QSqlDatabase db = QSqlDatabase::database("productitemConnection");
    if(db.isOpen()) {
        productModel->submitAll();
        db.close();
    }
}

int ProductManagerForm::makePId( )
{
    if(productModel->rowCount() == 0) {                                               // clientList의 데이터가 없다면
        return 8000;                                                            // id를 1000번부터 부여
    } else {
        int lastNum = productModel->rowCount();
        auto pid = productModel->data(productModel->index(lastNum-1, 0)).toInt();             // clientList의 마지막값을 id로 가져와
        return ++pid;                                                            // +1 하여 아이디 부여
    }
}

void ProductManagerForm::removeItem()
{
    QModelIndex model = ui->producttreeView->currentIndex();                     // 선택된 항목을 저장
    QSqlDatabase db = QSqlDatabase::database("productitemConnection");
    if(db.isOpen() && model.isValid()) {
        productModel->removeRow(model.row());
        productModel->select();
        for(int i=0;i<productModel->columnCount();i++){
            ui->producttreeView->resizeColumnToContents(i);
        }
    }
}

void ProductManagerForm::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = ui->producttreeView->mapToGlobal(pos);                  // 우클릭된 모니터의 좌표값 불러오기
    if(ui->producttreeView->indexAt(pos).isValid())
        menu->exec(globalPos);
}

void ProductManagerForm::on_searchPushButton_clicked()
{
    sproductModel->clear();
    int i = ui->searchComboBox->currentIndex();                       // 검색할 id,이름,전화번호, 타입을 설정
    auto flag = (i)? Qt::MatchFixedString|Qt::MatchContains
                   : Qt::MatchCaseSensitive;
    QModelIndexList indexes = productModel->match(productModel->index(0, i), Qt::EditRole,
                                                  ui->searchLineEdit->text(),
                                                  -1, Qt::MatchFlags(flag));

    foreach(auto idx, indexes) {
        int pid = productModel->data(idx.siblingAtColumn(0)).toInt(); //c->id();
        QString pname = productModel->data(idx.siblingAtColumn(1)).toString();
        int price = productModel->data(idx.siblingAtColumn(2)).toInt();
        int stock = productModel->data(idx.siblingAtColumn(3)).toInt();
        QStringList strings;
        strings << QString::number(pid) << pname << QString::number(price) << QString::number(stock);

        QList<QStandardItem *> items;
        for (int i = 0; i < 4; ++i) {
            items.append(new QStandardItem(strings.at(i)));
        }

        sproductModel->appendRow(items);
        sproductModel->setHeaderData(0, Qt::Horizontal, tr("PID"));
        sproductModel->setHeaderData(1, Qt::Horizontal, tr("PName"));
        sproductModel->setHeaderData(2, Qt::Horizontal, tr("Price"));
        sproductModel->setHeaderData(3, Qt::Horizontal, tr("Stock"));
        ui->searchTreeView->setModel(sproductModel);
        for(int i=0;i<sproductModel->columnCount();i++){
            ui->searchTreeView->resizeColumnToContents(i);
        }
    }
}

void ProductManagerForm::on_modifyPushButton_clicked()
{
    QModelIndex model = ui->producttreeView->currentIndex();
    QSqlDatabase db = QSqlDatabase::database("productitemConnection");
    if(db.isOpen()){
        int key = model.sibling(model.row(),0).data().toInt();                                        // 0번째항목에서 key값추출
        QString pname, price, stock;                                          // 이름,전화번호,주소 QString형 변수 생성
        pname = ui->pnameLineEdit->text();                                        // nameLineEdit의 text를 name변수에 저장
        price = ui->priceLineEdit->text();                               // phoneNumberLineEdit의 text를 number변수에 저장
        stock = ui->stockLineEdit->text();                                  // addressLineEdit의 text를 address변수에 저장

        productModel->setData(model.siblingAtColumn(1), pname);
        productModel->setData(model.siblingAtColumn(2), price);
        productModel->setData(model.siblingAtColumn(3), stock);
        productModel->submit();
    }
}


void ProductManagerForm::on_addPushButton_clicked()
{
    QString pname, price, stock;                                              // 이름,전화번호,주소 QString형 변수 생성
    int pid = makePId();                                                         // id생성후 id변수에 저장
    pname = ui->pnameLineEdit->text();                                            // nameLineEdit에 text를 name변수에 저장
    price = ui->priceLineEdit->text();                                   // phoneNumberLineEdit의 text를 number변수에 저장
    stock = ui->stockLineEdit->text();                                      // addressLineEdit의 text를 address변수에 저장
    QSqlDatabase db = QSqlDatabase::database("productitemConnection");
    if(db.isOpen() && pname.length()) {
        QSqlQuery query(productModel->database());
        query.prepare("INSERT INTO productitem VALUES (?, ?, ?, ?)");
        query.bindValue(0, pid);
        query.bindValue(1, pname);
        query.bindValue(2, price);
        query.bindValue(3, stock);
        query.exec();
        productModel->select();
    }
}

void ProductManagerForm::on_deletePushButton_clicked()
{
    removeItem();
}

void ProductManagerForm::productIdListData(int index)
{
    QString PIdstr;
    QList<QString> PIdList;

    for(int i = 0; i < productModel->rowCount(); i++) {
        QString pid = productModel->data(productModel->index(i, 0)).toString();
        QString pname = productModel->data(productModel->index(i, 1)).toString();
        PIdstr = QString("%1, %2").arg(pid).arg(pname);                             // "ID, 이름" 형태의 QString형 변수 저장
        PIdList.append(PIdstr);
    }

    emit productDataListSent(PIdList);
}

void ProductManagerForm::productNameListData(QString pname)
{
    QSqlDatabase db = QSqlDatabase::database("productitemConnection");
    QSqlTableModel query(this,db);
    query.QSqlQueryModel::setQuery(QString("select * "
                                           "from productitem "
                                           "where p_name "
                                           "like '%%1%' "
                                           "order by p_id").arg(pname),db);
    for(int i = 0; i < query.rowCount(); i++) {
        int pid = query.data(productModel->index(i, 0)).toInt();
        QString pname = query.data(productModel->index(i, 1)).toString();
        int price = query.data(productModel->index(i, 2)).toInt();
        int stock = query.data(productModel->index(i, 3)).toInt();
        emit productFindDataSent(pid,pname,price,stock);
    }
}

void ProductManagerForm::productItemRecv(int pid)
{
    QSqlDatabase db = QSqlDatabase::database("productitemConnection");
    QSqlTableModel query(this,db);
    query.QSqlQueryModel::setQuery(QString("select * "
                                           "from productitem "
                                           "where p_id = '%1'").arg(pid),db);
    QString pname = query.data(query.index(0, 1)).toString();
    QString price = query.data(query.index(0, 2)).toString();
    QString stock = query.data(query.index(0, 3)).toString();

    emit productIdDataSent(pid,pname,price,stock);
}

void ProductManagerForm::productStockUp(int pid,int amount)
{
    QSqlDatabase db = QSqlDatabase::database("productitemConnection");
    QSqlTableModel query(this,db);
    query.QSqlQueryModel::setQuery(QString("select * "
                                           "from productitem "
                                           "where p_id = '%1'").arg(pid),db);
    int updateStock = query.data(query.index(0, 3)).toInt() - amount;
    query.QSqlQueryModel::setQuery(QString("update productitem "
                                           "set p_stock = '%1'"
                                           "where p_id = '%2'").arg(updateStock).arg(pid),db);
    query.submit();
    productModel->QSqlQueryModel::setQuery(QString("select * "
                                                   "from productitem"),db);
    productModel->submit();

    emit udstockSend(updateStock);
}

void ProductManagerForm::on_producttreeView_clicked(const QModelIndex &index)
{
    Q_UNUSED(index);                                                                // column인자 사용안함
    QModelIndex model = ui->producttreeView->currentIndex();
    ui->idLineEdit->setText(model.sibling(model.row(),0).data().toString());        // idLineEdit에 item의 text(0)값을 저장
    ui->pnameLineEdit->setText(model.sibling(model.row(),1).data().toString());                                   // nameLineEdit에 item의 text(1)값을 저장
    ui->priceLineEdit->setText(model.sibling(model.row(),2).data().toString());                            // phoneNumberLineEdit item의 text(2)값을 저장
    ui->stockLineEdit->setText(model.sibling(model.row(),3).data().toString());                                // addressLineEdit에 item의 text(3)값을 저장
    ui->toolBox->setCurrentIndex(0);
}


void ProductManagerForm::on_searchTreeView_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);                                                           // column인자 사용안함
    QModelIndex model = ui->searchTreeView->currentIndex();
    ui->idLineEdit->setText(model.sibling(model.row(),0).data().toString());                                     // idLineEdit에 item의 text(0)값을 저장
    ui->pnameLineEdit->setText(model.sibling(model.row(),1).data().toString());                                   // nameLineEdit에 item의 text(1)값을 저장
    ui->priceLineEdit->setText(model.sibling(model.row(),2).data().toString());                            // phoneNumberLineEdit item의 text(2)값을 저장
    ui->stockLineEdit->setText(model.sibling(model.row(),3).data().toString());                                // addressLineEdit에 item의 text(3)값을 저장
    ui->toolBox->setCurrentIndex(0);
}

