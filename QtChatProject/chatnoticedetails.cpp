#include "chatnoticedetails.h"
#include "ui_chatnoticedetails.h"

ChatNoticeDetails::ChatNoticeDetails(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatNoticeDetails)
{
    ui->setupUi(this);
    QList<int> sizes;
    sizes << 300 << 600;
    ui->Nsplitter->setSizes(sizes);

    connect(ui->quitpushButton, SIGNAL(clicked()), SLOT(close));
}

ChatNoticeDetails::~ChatNoticeDetails()
{
    qDebug() << "close";
    delete ui;
}

void ChatNoticeDetails::closeEvent(QCloseEvent*)
{
    qDebug() << "close";
    emit closeNotice();
}

void ChatNoticeDetails::setClientList(QHash<quint16, QString> clientNameHash)
{
    QStringList strings;
    QHash<quint16, QString>::iterator i;
    for (i = clientNameHash.begin(); i != clientNameHash.end(); ++i) {
        strings << QString::number(i.key()) << i.value();
        chatList.append(new QTreeWidgetItem(strings));
        strings.clear();
    }
    ui->clientTreeWidget->addTopLevelItems(chatList);
    ui->clientTreeWidget->resizeColumnToContents(0);
}
