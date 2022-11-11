#ifndef CHATNOTICEDETAILS_H
#define CHATNOTICEDETAILS_H

#include "qtreewidget.h"
#include <QWidget>

namespace Ui {
class ChatNoticeDetails;
}

class ChatNoticeDetails : public QWidget
{
    Q_OBJECT

public:
    explicit ChatNoticeDetails(QWidget *parent = nullptr);
    ~ChatNoticeDetails();

    void setClientList(QHash<quint16, QString>);

protected:
    void closeEvent(QCloseEvent*) override;

signals:
    void closeNotice();

private:
    Ui::ChatNoticeDetails *ui;

    QList<QTreeWidgetItem*> chatList;
};

#endif // CHATNOTICEDETAILS_H
