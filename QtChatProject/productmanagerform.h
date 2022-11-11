#ifndef PRODUCTMANAGERFORM_H
#define PRODUCTMANAGERFORM_H

#include <QWidget>
#include <QHash>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>

#include "productitem.h"

class ProductItem;
class QMenu;
class QTreeWidgetItem;

namespace Ui {
class ProductManagerForm;
}

class ProductManagerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ProductManagerForm(QWidget *parent = nullptr);
    ~ProductManagerForm();
    void loadData();
public slots:
    void productIdListData(int);
    void productNameListData(QString);
    void productItemRecv(int);
    void productStockUp(int,int);

private slots:
    void showContextMenu(const QPoint &);
    void removeItem();

    void on_searchPushButton_clicked();

    void on_modifyPushButton_clicked();

    void on_addPushButton_clicked();

    void on_deletePushButton_clicked();

    void on_producttreeView_clicked(const QModelIndex &index);

    void on_searchTreeView_doubleClicked(const QModelIndex &index);

signals:
    void productAdded(QString);
    void productDataListSent(QList<QString>);
    void productFindDataSent(int,QString,int,int);
    void productIdDataSent(int,QString,QString,QString);

    void udstockSend(int);

private:
    int makePId();

    QMap<int, ProductItem*> productList;
    Ui::ProductManagerForm *ui;
    QMenu* menu;

    QSqlTableModel* productModel;
    QSqlTableModel* sproductModel;
};

#endif // PRODUCTMANAGERFORM_H
