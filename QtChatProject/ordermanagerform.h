#ifndef ORDERMANAGERFORM_H
#define ORDERMANAGERFORM_H

#include "clientitem.h"
#include "productitem.h"
#include <QWidget>
#include <QHash>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>

#include "orderitem.h"

class OrderItem;
class QMenu;
class QTreeWidgetItem;
class ClientManagerForm;
class ProductManagerForm;

namespace Ui {
class OrderManagerForm;
}

class OrderManagerForm : public QWidget
{
    Q_OBJECT

public:
    explicit OrderManagerForm(QWidget *parent = nullptr);
    ~OrderManagerForm();
    void loadData();

public slots:

    void clientDataListRecv(QList<QString>);
    void clientFindDataRecv(int,QString,QString,QString);

    void productDataListRecv(QList<QString>);
    void productFindDataRecv(int,QString,int,int);
    void getProductIdDataRecv(ProductItem*);

    void udstockRecv(int);

private slots:
    void showContextMenu(const QPoint &);
    void removeItem();

    void on_clientcomboBox_currentIndexChanged(int index);

    void on_clientInfocomboBox_textActivated(const QString &arg1);

    void on_clienttreeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_productcomboBox_currentIndexChanged(int index);

    void on_productInfocomboBox_textActivated(const QString &arg1);

    void on_producttreeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_addpushButton_clicked();

    void on_amountspinBox_valueChanged(int arg1);

    void on_ordertreeWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_deletepushButton_clicked();

    void on_modifypushButton_clicked();

signals:
    void orderAdded(QString);

    void clientDataSent(int);
    void clientDataSent(QString);

    void productDataSent(int);
    void productDataSent(QString);
    void getProductItemSent(int);
    void stockUpdate(int,int);

private:
    int makeOId();

    QMap<int, OrderItem*> orderList;
    Ui::OrderManagerForm *ui;
    QMenu* menu;

    QSqlTableModel* orderModel;
    QSqlTableModel* scoModel;
    QSqlTableModel* spoModel;
};

#endif // ORDERMANAGERFORM_H
