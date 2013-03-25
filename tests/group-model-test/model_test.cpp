#include "model_test.h"

#include <QStandardItem>
#include <QStandardItemModel>
#include <KTp/types.h>

QStandardItem* createContact(const QString &name, const QStringList &groups) {
    QStandardItem *item = new QStandardItem(name);

    item->setData(KTp::ContactRowType, KTp::RowTypeRole);
    item->setData(name, KTp::IdRole);

    item->setData(QStringList() << "pc" ,KTp::ContactClientTypesRole);
    item->setData(QString() ,KTp::ContactAvatarPathRole);
    item->setData(groups ,KTp::ContactGroupsRole);

    setPresence(item, Tp::ConnectionPresenceTypeAvailable);

    item->setData(true ,KTp::ContactSubscriptionStateRole);
    item->setData(true ,KTp::ContactPublishStateRole);
    item->setData(true ,KTp::ContactIsBlockedRole);

    item->setData(false ,KTp::ContactHasTextChannelRole);
    item->setData(0 ,KTp::ContactUnreadMessageCountRole);

    item->setData(true ,KTp::ContactCanTextChatRole);
    item->setData(true ,KTp::ContactCanFileTransferRole);
    item->setData(true ,KTp::ContactCanAudioCallRole);
    item->setData(true ,KTp::ContactCanVideoCallRole);
    item->setData(QStringList() ,KTp::ContactTubesRole);

    return item;
}

QStandardItem* createMetaContact(QList<QStandardItem*> contacts) {
    QStandardItem *item = new QStandardItem(contact.first()->data(KTp::IdRole));

    item->setData(KTp::PersonRowType, KTp::RowTypeRole);
    item->setData(contact.first()->data(KTp::IdRole), KTp::IdRole);

    item->setData(QStringList() << "pc" ,KTp::ContactClientTypesRole);
    item->setData(QString() ,KTp::ContactAvatarPathRole);

    setPresence(item, Tp::ConnectionPresenceTypeAvailable);

    item->setData(true ,KTp::ContactSubscriptionStateRole);
    item->setData(true ,KTp::ContactPublishStateRole);
    item->setData(true ,KTp::ContactIsBlockedRole);

    item->setData(false ,KTp::ContactHasTextChannelRole);
    item->setData(0 ,KTp::ContactUnreadMessageCountRole);

    item->setData(true ,KTp::ContactCanTextChatRole);
    item->setData(true ,KTp::ContactCanFileTransferRole);
    item->setData(true ,KTp::ContactCanAudioCallRole);
    item->setData(true ,KTp::ContactCanVideoCallRole);
    item->setData(QStringList() ,KTp::ContactTubesRole);

    QStringList groups;

    Q_FOREACH(QStandardItem *childContact, contacts) {
        groups << childContact->data(KTp::ContactGroupsRole).toString();
        item->appendRow(childContact);
    }

    item->setData(groups ,KTp::ContactGroupsRole);

    return item;
}

void setPresence(QStandardItem *item, Tp::ConnectionPresenceType presence)
{
    KTp::Presence p(Tp::Presence(presence, "", ""));

    item->setData(p.displayString() ,KTp::ContactPresenceNameRole);
    item->setData(p.statusMessage() ,KTp::ContactPresenceMessageRole);
    item->setData(p.type() ,KTp::ContactPresenceTypeRole);
    item->setData(p.iconName() ,KTp::ContactPresenceIconRole);
}



model_test::model_test(QObject *parent) :
    QObject(parent)
{
    QStandardItemModel *m_contactList = new QStandardItemModel(this);

    QStandardItem *david = createContact("David", QStringList() << "kde" << "uk");
    QStandardItem *martin = createContact("Martin", QStringList() << "kde" << "czech");
    QStandardItem *dan = createContact("Dan", QStringList() << "kde" << "czech");
    QStandardItem *rohan = createContact("Rohan", QStringList());
    
    QStandardItem *meta1 = createMetaContact(QList<QStandardItem*>() << martin << dan);

    m_contactList->appendRow(david);
//     m_contactList->appendRow(martin);
//     m_contactList->appendRow(dan);
//     m_contactList->appendRow(andrew);
    m_contactList->appendRow(rohan);
    m_contactList->appendRow(meta1);
}


/*groups should currently be:
czech:
 - dan
 - marin
kde:
 - dan
 - david
 - martin
uk:
 - david
ungrouped:
 - rohan
*/




