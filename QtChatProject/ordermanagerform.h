#ifndef ORDERMANAGERFORM_H
#define ORDERMANAGERFORM_H

#include <QWidget>
#include <QHash>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>

class QMenu;                                                            // QMenu클래스 참조
class QTreeWidgetItem;                                                  // QTreeWidgetItem클래스 참조
class ClientManagerForm;                                                // ClientManagerForm클래스 참조
class ProductManagerForm;                                               // ProductManagerForm클래스 참조

namespace Ui {
class OrderManagerForm;
}

class OrderManagerForm : public QWidget
{
    Q_OBJECT

public:
    explicit OrderManagerForm(QWidget *parent = nullptr);               // OrderManagerForm생성자
    ~OrderManagerForm();                                                // OrderManagerForm소멸자
    void loadData();                                                    // QSQLITE를 DB로 사용 및 orderitemConnection으로 DB접속선언

public slots:
    void clientDataListRecv(QList<QString>);                            // Client DB에서 뽑은 cid, cname의 QList형으로 받기위한 슬롯
    void clientFindDataRecv(int,QString,QString,QString);               // ClientItem에서 검색된 값을 받기위한 슬롯

    void productDataListRecv(QList<QString>);                           // product DB에서 뽑은 pid, pname의 QList형으로 받기위한 슬롯
    void productFindDataRecv(int,QString,int,int);                      // productList에서 검색된 값을 받기위한 슬롯
    void getProductIdDataRecv(int,QString,QString,QString);             // stock의 값을 최신화하기위한 슬롯

    void udstockRecv(int);                                              // update된 stock값을 받기위한 슬롯함수

private slots:
    void showContextMenu(const QPoint &);                               // ordertreeView내부에서 우클릭동작시 ContextMenu를 실행하기 위함
    void removeItem();                                                  // treeView의 항목 제거용

    void on_clientcomboBox_currentIndexChanged(int index);              // clientcomboBox의 index변경시 실행되는 슬롯
    void on_clientInfocomboBox_textActivated(const QString &arg1);      // clientInfocomboBox의 text가 활성화시 실행되는 슬롯
    void on_clienttreeWidget_itemDoubleClicked(QTreeWidgetItem *item,   // clienttreeWidget의 항목이 클릭시 실행되는 슬롯
                                               int column);
    void on_productcomboBox_currentIndexChanged(int index);             // productcomboBox의 index변경시 실행되는 슬롯
    void on_productInfocomboBox_textActivated(const QString &arg1);     // productInfocomboBox의 text가 활성화시 실행되는 슬롯
    void on_producttreeWidget_itemDoubleClicked(QTreeWidgetItem *item,  // producttreeWidget의 항목이 클릭시 실행되는 슬롯
                                                int column);
    void on_addpushButton_clicked();                                    // Add버튼 - Item추가
    void on_amountspinBox_valueChanged(int arg1);                       // amountspinBox의 값이 바뀌면 실행되는 슬롯
    void on_deletepushButton_clicked();                                 // Delete버튼 - Item삭제
    void on_modifypushButton_clicked();                                 // Modify버튼 - Item수정
    void on_ordertreeView_clicked(const QModelIndex &index);            // ordertreeView 클릭시

signals:
    void clientDataSent(int);                                           // index값을 clientmanager로 보내기위한 signal
    void clientDataSent(QString);                                       // name을 clientManager로 보내기위한 signal

    void productDataSent(int);                                          // index값을 productmanager로 보내기위한 signal
    void productDataSent(QString);                                      // name을 productManager로 보내기위한 signal
    void getProductItemSent(int);                                       // productID를 Productitem으로 보내기위한 signal
    void stockUpdate(int,int);                                          // product의 stock 업데이트를위한 signal

private:
    int makeOId();                                                      // Order Id를 부여하기위한 멤버함수

    Ui::OrderManagerForm *ui;                                           // Ui를 사용하기위한 멤버변수
    QMenu* menu;                                                        // Action을 동작하기위한 메뉴생성 멤버변수

    QSqlTableModel* orderModel;                                         // ordertreeview에 보여주기위한 base 모델
};

#endif // ORDERMANAGERFORM_H
