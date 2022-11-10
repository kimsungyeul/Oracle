#include "ordermanagerform.h"
#include "orderitem.h"
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

    QList<int> sizes;
    sizes << 320 << 600;
    ui->splitter->setSizes(sizes);

    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));

    menu = new QMenu;
    menu->addAction(removeAction);
    ui->ordertreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->ordertreeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
}

void OrderManagerForm::loadData()
{
//    QFile file("orderlist.txt");
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
//        return;

//    QTextStream in(&file);
//    while (!in.atEnd()) {
//        QString line = in.readLine();
//        QList<QString> row = line.split(",");
//        if(row.size()) {
//            int oid = row[0].toInt();
//            int cid = row[1].toInt();
//            QString cname = row[2];
//            int pid = row[3].toInt();
//            QString pname = row[4];
//            int amount = row[5].toInt();
//            int totprice = row[6].toInt();
//            QString date = row[7];

//            OrderItem* o = new OrderItem(oid, cid, cname, pid, pname, amount, totprice, date);
//            //ui->ordertreeWidget->addTopLevelItem(o);
//            orderList.insert(oid, o);

//            QTreeWidgetItem* orderitem = new QTreeWidgetItem(ui->ordertreeWidget);
//            QString CNameIdstr, PNameIdstr;
//            CNameIdstr = QString("%1(%2)").arg(cname).arg(cid);
//            PNameIdstr = QString("%1(%2)").arg(pname).arg(pid);

//            orderitem->setText(0,QString::number(oid));
//            orderitem->setText(1, CNameIdstr);
//            orderitem->setText(2, PNameIdstr);
//            orderitem->setText(3,QString::number(amount));
//            orderitem->setText(4,QString::number(totprice));
//            orderitem->setText(5,date);
//        }
//    }
//    file.close( );
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "orderitemConnection");
    db.setDatabaseName("orderitem.db");
    if (db.open()) {
        QSqlQuery query(db);
        query.exec("CREATE TABLE IF NOT EXISTS orderitem("
                   "o_id INTEGER Primary Key, "
                   "o_c_id INTEGER,"
                   "o_c_name VARCHAR(30) NOT NULL, "
                   "o_p_id INTEGER,"
                   "o_p_name VARCHAR(30) NOT NULL, "
                   "o_amount INTEGER, "
                   "o_totprice INTEGER, "
                   "o_date date NOT NULL);");

        orderModel = new QSqlTableModel(this, db);
        orderModel->setTable("orderitem");
        orderModel->select();
        orderModel->setHeaderData(0, Qt::Horizontal, tr("OID"));
        orderModel->setHeaderData(1, Qt::Horizontal, tr("CID"));
        orderModel->setHeaderData(2, Qt::Horizontal, tr("CName"));
        orderModel->setHeaderData(3, Qt::Horizontal, tr("PID"));
        orderModel->setHeaderData(4, Qt::Horizontal, tr("PName"));
        orderModel->setHeaderData(5, Qt::Horizontal, tr("Amount"));
        orderModel->setHeaderData(6, Qt::Horizontal, tr("Total Price"));
        orderModel->setHeaderData(7, Qt::Horizontal, tr("Date"));

        ui->ordertreeView->setModel(orderModel);

        ui->ordertreeView->setRootIsDecorated(false);

        for(int i=0;i<orderModel->columnCount();i++){
            ui->ordertreeView->resizeColumnToContents(i);
        }
    }
}

OrderManagerForm::~OrderManagerForm()
{
//    delete ui;

//    QFile file("orderlist.txt");
//    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
//        return;

//    QTextStream out(&file);
//    for (const auto& v : orderList) {
//        OrderItem* o = v;
//        out << o->oid() << "," << o->getcid() << "," ;
//        out << o->getcname() << "," << o->getpid() << ",";
//        out << o->getpname() << "," << o-> getAmount()<< ",";
//        out << o->getTotPrice() << ",";
//        out << o->getDate() << "\n";
//    }
//    file.close( );
    delete ui;
    QSqlDatabase db = QSqlDatabase::database("orderitemConnection");
    if(db.isOpen()) {
        orderModel->submitAll();
        db.close();
    }
}

int OrderManagerForm::makeOId( )
{
    if(orderModel->rowCount() == 0) {                                               // clientList의 데이터가 없다면
        return 5000;                                                            // id를 1000번부터 부여
    } else {
        int lastNum = orderModel->rowCount();
        auto oid = orderModel->data(orderModel->index(lastNum-1, 0)).toInt();             // clientList의 마지막값을 id로 가져와
        return ++oid;                                                            // +1 하여 아이디 부여
    }
}

void OrderManagerForm::removeItem()
{
    QModelIndex model = ui->ordertreeView->currentIndex();                     // 선택된 항목을 저장
    QSqlDatabase db = QSqlDatabase::database("orderitemConnection");
    if(db.isOpen() && model.isValid()) {
        //clientList.remove(clientModel->data(index.siblingAtColumn(0)).toInt());
        orderModel->removeRow(model.row());
        orderModel->select();
    }
}

void OrderManagerForm::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = ui->ordertreeView->mapToGlobal(pos);                  // 우클릭된 모니터의 좌표값 불러오기
    if(ui->ordertreeView->indexAt(pos).isValid())
        menu->exec(globalPos);
}

void OrderManagerForm::on_clientcomboBox_currentIndexChanged(int index)
{
    ui->clientInfocomboBox->clear();

    if (index == 0) {
        ui->clientInfocomboBox->setEditable(false);
        ui->clienttreeWidget->clear();
        ui->clientInfocomboBox->clear();
    }
    else if (index == 1) {
        ui->clientInfocomboBox->setEditable(false);
        emit clientDataSent(index);
    }
    else if (index == 2) {
        ui->clientInfocomboBox->setEditable(true);
        ui->clientInfocomboBox->setMaxVisibleItems(5);
    }
}

void OrderManagerForm::on_clientInfocomboBox_textActivated(const QString &arg1)
{
    ui->clienttreeWidget->clear();

    if(ui->clientcomboBox->currentIndex() == 1){
        QList<QString> row = arg1.split(", ");
        emit clientDataSent(row[1]);
    }
    else if(ui->clientcomboBox->currentIndex() == 2){
        emit clientDataSent(arg1);
    }
}

void OrderManagerForm::clientFindDataRecv(int id,QString name,QString number,QString address)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->clienttreeWidget);
    if(id != NULL) {
        item->setText(0,QString::number(id));
        item->setText(1,name);
        item->setText(2,number);
        item->setText(3,address);
    }
    for(int i = 0; i < ui->clienttreeWidget->columnCount(); i++){
        ui->clienttreeWidget->resizeColumnToContents(i);
    }
}



void OrderManagerForm::clientDataListRecv(QList<QString> IdList)
{
    ui->clientInfocomboBox->addItems(IdList);
}

void OrderManagerForm::on_clienttreeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    QString NameIdstr;
    int cid = item->text(0).toInt();
    QString name = item->text(1);
    NameIdstr = QString("%1(%2)").arg(name).arg(cid);

    if(item != nullptr) {
        ui->nameLineEdit->setText(NameIdstr);
    }
}

void OrderManagerForm::on_productcomboBox_currentIndexChanged(int index)
{
    ui->productInfocomboBox->clear();

    if (index == 0) {
        ui->productInfocomboBox->setEditable(false);
        ui->producttreeWidget->clear();
        ui->productInfocomboBox->clear();
    }
    else if (index == 1) {
        ui->productInfocomboBox->setEditable(false);
        emit productDataSent(index);
    }
    else if (index == 2) {
        ui->productInfocomboBox->setEditable(true);
        ui->productInfocomboBox->setMaxVisibleItems(5);
    }
}

void OrderManagerForm::on_productInfocomboBox_textActivated(const QString &arg1)
{
    ui->producttreeWidget->clear();

    if(ui->productcomboBox->currentIndex() == 1){
        QList<QString> row = arg1.split(", ");
        emit productDataSent(row[1]);
    }
    else if(ui->productcomboBox->currentIndex() == 2){
        emit productDataSent(arg1);
    }
}

void OrderManagerForm::productFindDataRecv(int pid,QString pname,int price ,int stock)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->producttreeWidget);
    if(pid != NULL) {
        item->setText(0,QString::number(pid));
        item->setText(1,pname);
        item->setText(2,QString::number(price));
        item->setText(3,QString::number(stock));
    }
    for(int i = 0; i < ui->producttreeWidget->columnCount(); i++){
        ui->producttreeWidget->resizeColumnToContents(i);
    }
}

void OrderManagerForm::productDataListRecv(QList<QString> PIdList)
{
    ui->productInfocomboBox->addItems(PIdList);
}

void OrderManagerForm::on_producttreeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    QString NameIdstr;
    int pid = item->text(0).toInt();
    QString pname = item->text(1);
    int stock = item->text(3).toInt();
    NameIdstr = QString("%1(%2)").arg(pname).arg(pid);

    if(item != nullptr) {
        ui->pnameLineEdit->setText(NameIdstr);
        ui->priceLineEdit->setText(item->text(2));
        ui->stockLineEdit->setText(item->text(3));
        ui->amountspinBox->setMaximum(stock);
    }
}

void OrderManagerForm::on_addpushButton_clicked()
{
    //QTreeWidgetItem* row = new QTreeWidgetItem(ui->ordertreeWidget);
    QString citemname = ui->nameLineEdit->text();
    QString pitemname = ui->pnameLineEdit->text();
    // Order Number
    if((citemname.isEmpty() != true) && (pitemname.isEmpty() != true)) {
        int oid = makeOId();
        QList<QString> clientrow = ui->nameLineEdit->text().split("(");
        QList<QString> productrow = ui->pnameLineEdit->text().split("(");
        clientrow[1] = clientrow[1].remove(QChar(')'), Qt::CaseInsensitive);
        productrow[1] = productrow[1].remove(QChar(')'), Qt::CaseInsensitive);
        // Client Name, Item Name
        int cid = clientrow[1].toInt();
        QString cname = clientrow[0];
        int pid = productrow[1].toInt();
        QString pname = productrow[0];
        int amount = ui->amountspinBox->text().toInt();
        int totprice = ui->totalpriceLineEdit->text().toInt();
        // Date
        QString time_format = "yyyy-MM-dd HH:mm:ss";
        QString date = QDateTime::currentDateTime().toString(time_format);

        QSqlDatabase db = QSqlDatabase::database("orderitemConnection");
        if(db.isOpen()) {
            QSqlQuery query(orderModel->database());
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
            for(int i=0;i<orderModel->columnCount();i++){
                ui->ordertreeView->resizeColumnToContents(i);
            }
            emit stockUpdate(pid,amount);
        }
    } else {
        QMessageBox::critical(this, tr("Invalid Order"), \
                              tr("PlZ Client or Product input"));
    }
}


void OrderManagerForm::on_amountspinBox_valueChanged(int arg1)
{
    ui->totalpriceLineEdit->setText(QString::number(arg1 * ui->priceLineEdit->text().toInt()));
}

void OrderManagerForm::getProductIdDataRecv(ProductItem* item)
{
//    ui->producttreeWidget->clear();

//    //ui->producttreeWidget->addTopLevelItem(item);
//    QString maxstock = item->text(3);
//    ui->priceLineEdit->setText(item->text(2));
//    ui->amountspinBox->setMaximum(maxstock.toInt());
//    ui->stockLineEdit->setText(maxstock);
}

void OrderManagerForm::on_ordertreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
//    Q_UNUSED(column);

//    QList<QString> clientrow = item->text(1).split("(");
//    QList<QString> productrow = item->text(2).split("(");
//    int clientID = clientrow[1].remove(QChar(')'), Qt::CaseInsensitive).toInt();
//    int productID = productrow[1].remove(QChar(')'), Qt::CaseInsensitive).toInt();

//    emit getProductItemSent(productID);

//    ui->nameLineEdit->setText(item->text(1));
//    ui->pnameLineEdit->setText(item->text(2));
//    ui->totalpriceLineEdit->setText(item->text(4));
}


void OrderManagerForm::on_deletepushButton_clicked()
{
    removeItem();
}


void OrderManagerForm::on_modifypushButton_clicked()
{
//    QTreeWidgetItem* item = ui->ordertreeWidget->currentItem();
//    if(item != nullptr) {
//        int key = item->text(0).toInt();
//        OrderItem* o = orderList[key];

//        int befamount, afamount, totalPrice;
//        QString cname,pname;
// //유나코드 해야함
////        cid = clientIDList[ui->clientComboBox->currentIndex()];
////        pid = productIDList[ui->productComboBox->currentIndex()];
//        QList<QString> productrow = ui->pnameLineEdit->text().split("(");
//        productrow[1] = productrow[1].remove(QChar(')'), Qt::CaseInsensitive);
//        int pid = productrow[1].toInt();

//        befamount = item->text(3).toInt();
//        afamount = ui->amountspinBox->value();
//        totalPrice = ui->totalpriceLineEdit->text().toInt();

//        item->setText(3,QString::number(afamount));
//        item->setText(4,QString::number(totalPrice));
//        o->setAmount(afamount);
//        o->setTotPrice(totalPrice);
//        orderList[key] = o;
//        emit stockUpdate(pid,afamount-befamount);
//    }
}

void OrderManagerForm::udstockRecv(int stock)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->producttreeWidget->currentItem());
    if(item != NULL) {
        item->setText(3,QString::number(stock));
        ui->producttreeWidget->update();
    }
}
