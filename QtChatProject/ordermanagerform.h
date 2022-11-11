#ifndef ORDERMANAGERFORM_H
#define ORDERMANAGERFORM_H

#include <QWidget>
#include <QHash>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>

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
    void getProductIdDataRecv(int,QString,QString,QString);

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

    void on_deletepushButton_clicked();

    void on_modifypushButton_clicked();

    void on_ordertreeView_clicked(const QModelIndex &index);

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

    Ui::OrderManagerForm *ui;
    QMenu* menu;

    QSqlTableModel* orderModel;
    QSqlTableModel* scoModel;
    QSqlTableModel* spoModel;
};

#endif // ORDERMANAGERFORM_H
