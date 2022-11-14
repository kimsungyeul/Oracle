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

    connect(ui->quitpushButton, SIGNAL(clicked()), SLOT(close()));
}

ChatNoticeDetails::~ChatNoticeDetails()
{
    delete ui;
}

void ChatNoticeDetails::closeEvent(QCloseEvent*)
{
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

void ChatNoticeDetails::on_sendpushButton_clicked()
{
    QString str = ui->noticelineEdit->text();
    if(str.length()){
        QByteArray bytearray;
        //bytearray.clear();
        bytearray = str.toUtf8( );
        /* 화면에 표시 : 앞에 '공지사항'라고 추가 */
        ui->noticetextEdit->append("<font color=red>공지사항</font> : " + str);
    }
    emit sendNoticeData(str);
}

