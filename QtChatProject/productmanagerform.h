#ifndef PRODUCTMANAGERFORM_H
#define PRODUCTMANAGERFORM_H

#include <QWidget>
#include <QHash>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>
#include <QStandardItemModel>

class QMenu;                                                        // QMenu클래스 참조
class QTreeWidgetItem;                                              // QTreeWidgetItem클래스 참조

namespace Ui {
class ProductManagerForm;
}

class ProductManagerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ProductManagerForm(QWidget *parent = nullptr);         // ProductManagerForm생성자
    ~ProductManagerForm();                                          // ProductManagerForm소멸자
    void loadData();                                                // QSQLITE를 DB로 사용 및 productitemConnection으로 DB접속선언

public slots:
    void productIdListData(int);                                    // ProductComoboBox의 Index에 따른 ProductList를 가져오기 위함
    void productNameListData(QString);                              // ProductComoboBox의 pname설정시 이름에따른 ProductList를 가져오기 위함
    void productItemRecv(int);                                      // ProductId를 받아 ProductDB의 item값을 추출하기위한 슬롯
    void productStockUp(int,int);                                   // OrderManagerForm에서 변경된 Stock의 pid와 amount를 받아 stock을 update하기위한 슬롯

private slots:
    void showContextMenu(const QPoint &);                           // treewidget내에서 우클릭시 action을 사용하기 위함
    void removeItem();                                              // treewidget의 항목 제거용

    void on_searchPushButton_clicked();                             // Search버튼 - Item검색
    void on_modifyPushButton_clicked();                             // Modify버튼 - Item수정
    void on_addPushButton_clicked();                                // Add버튼 - Item추가
    void on_deletePushButton_clicked();                             // Delete버튼 - Item삭제
    void on_producttreeView_clicked(const QModelIndex &index);      // TreeView의고객창 선택버튼 - 제품메뉴로 선택됨
    void on_searchTreeView_doubleClicked(const QModelIndex &index); // TreeView의검색창 선택버튼 - 검색메뉴로 선택됨

signals:
    void productDataListSent(QList<QString>);                       // ProductList를 OrderForm에 전달하기위한 시그널
    void productFindDataSent(int,QString,int,int);                  // 추출한 4개의 변수를 signal로 orderform으로 전달
    void productIdDataSent(int,QString,QString,QString);            // 추출한 4개의 변수를 signal로 orderform으로 전달

    void udstockSend(int);                                          // update된 값을 signal로 orderform으로 보냄

private:
    int makePId();                                                  // Product Id를 부여하기위한 멤버함수

    Ui::ProductManagerForm *ui;                                     // Ui를 사용하기위한 멤버변수
    QMenu* menu;                                                    // Action을 동작하기위한 메뉴생성 멤버변수

    QSqlTableModel* productModel;                                   // producttreeview에 보여주기위한 base 모델
    QStandardItemModel* sproductModel;                              // searchtreeview에 보여주기위한 사용자정의 검색모델
};

#endif // PRODUCTMANAGERFORM_H
