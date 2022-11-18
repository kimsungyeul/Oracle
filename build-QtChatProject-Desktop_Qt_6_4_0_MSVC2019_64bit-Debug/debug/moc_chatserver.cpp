/****************************************************************************
** Meta object code from reading C++ file 'chatserver.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../QtChatProject/chatserver.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'chatserver.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_ChatServer_t {
    uint offsetsAndSizes[38];
    char stringdata0[11];
    char stringdata1[14];
    char stringdata2[1];
    char stringdata3[12];
    char stringdata4[13];
    char stringdata5[10];
    char stringdata6[10];
    char stringdata7[16];
    char stringdata8[13];
    char stringdata9[8];
    char stringdata10[17];
    char stringdata11[11];
    char stringdata12[12];
    char stringdata13[16];
    char stringdata14[15];
    char stringdata15[47];
    char stringdata16[4];
    char stringdata17[45];
    char stringdata18[28];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_ChatServer_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_ChatServer_t qt_meta_stringdata_ChatServer = {
    {
        QT_MOC_LITERAL(0, 10),  // "ChatServer"
        QT_MOC_LITERAL(11, 13),  // "clientConnect"
        QT_MOC_LITERAL(25, 0),  // ""
        QT_MOC_LITERAL(26, 11),  // "receiveData"
        QT_MOC_LITERAL(38, 12),  // "removeClient"
        QT_MOC_LITERAL(51, 9),  // "addClient"
        QT_MOC_LITERAL(61, 9),  // "makeNonId"
        QT_MOC_LITERAL(71, 15),  // "NonMemaddClient"
        QT_MOC_LITERAL(87, 12),  // "inviteClient"
        QT_MOC_LITERAL(100, 7),  // "kickOut"
        QT_MOC_LITERAL(108, 16),  // "acceptConnection"
        QT_MOC_LITERAL(125, 10),  // "readClient"
        QT_MOC_LITERAL(136, 11),  // "privateChat"
        QT_MOC_LITERAL(148, 15),  // "privateChatSend"
        QT_MOC_LITERAL(164, 14),  // "noticeChatSend"
        QT_MOC_LITERAL(179, 46),  // "on_clientTreeWidget_customCon..."
        QT_MOC_LITERAL(226, 3),  // "pos"
        QT_MOC_LITERAL(230, 44),  // "on_chatTreeWidget_customConte..."
        QT_MOC_LITERAL(275, 27)   // "on_noticepushButton_clicked"
    },
    "ChatServer",
    "clientConnect",
    "",
    "receiveData",
    "removeClient",
    "addClient",
    "makeNonId",
    "NonMemaddClient",
    "inviteClient",
    "kickOut",
    "acceptConnection",
    "readClient",
    "privateChat",
    "privateChatSend",
    "noticeChatSend",
    "on_clientTreeWidget_customContextMenuRequested",
    "pos",
    "on_chatTreeWidget_customContextMenuRequested",
    "on_noticepushButton_clicked"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_ChatServer[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  110,    2, 0x0a,    1 /* Public */,
       3,    0,  111,    2, 0x0a,    2 /* Public */,
       4,    0,  112,    2, 0x0a,    3 /* Public */,
       5,    2,  113,    2, 0x0a,    4 /* Public */,
       6,    0,  118,    2, 0x0a,    7 /* Public */,
       7,    1,  119,    2, 0x0a,    8 /* Public */,
       8,    0,  122,    2, 0x0a,   10 /* Public */,
       9,    0,  123,    2, 0x0a,   11 /* Public */,
      10,    0,  124,    2, 0x0a,   12 /* Public */,
      11,    0,  125,    2, 0x0a,   13 /* Public */,
      12,    0,  126,    2, 0x0a,   14 /* Public */,
      13,    2,  127,    2, 0x0a,   15 /* Public */,
      14,    1,  132,    2, 0x0a,   18 /* Public */,
      15,    1,  135,    2, 0x08,   20 /* Private */,
      17,    1,  138,    2, 0x08,   22 /* Private */,
      18,    0,  141,    2, 0x08,   24 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,    2,    2,
    QMetaType::Int,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    2,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QPoint,   16,
    QMetaType::Void, QMetaType::QPoint,   16,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject ChatServer::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ChatServer.offsetsAndSizes,
    qt_meta_data_ChatServer,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_ChatServer_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ChatServer, std::true_type>,
        // method 'clientConnect'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'receiveData'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'removeClient'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'addClient'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'makeNonId'
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'NonMemaddClient'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'inviteClient'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'kickOut'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'acceptConnection'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'readClient'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'privateChat'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'privateChatSend'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'noticeChatSend'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'on_clientTreeWidget_customContextMenuRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPoint &, std::false_type>,
        // method 'on_chatTreeWidget_customContextMenuRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPoint &, std::false_type>,
        // method 'on_noticepushButton_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void ChatServer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ChatServer *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->clientConnect(); break;
        case 1: _t->receiveData(); break;
        case 2: _t->removeClient(); break;
        case 3: _t->addClient((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 4: { int _r = _t->makeNonId();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 5: _t->NonMemaddClient((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->inviteClient(); break;
        case 7: _t->kickOut(); break;
        case 8: _t->acceptConnection(); break;
        case 9: _t->readClient(); break;
        case 10: _t->privateChat(); break;
        case 11: _t->privateChatSend((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 12: _t->noticeChatSend((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->on_clientTreeWidget_customContextMenuRequested((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 14: _t->on_chatTreeWidget_customContextMenuRequested((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 15: _t->on_noticepushButton_clicked(); break;
        default: ;
        }
    }
}

const QMetaObject *ChatServer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ChatServer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ChatServer.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ChatServer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 16;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
