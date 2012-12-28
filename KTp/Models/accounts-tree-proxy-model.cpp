#include "accounts-tree-proxy-model.h"

#include "contacts-model.h"

#include <TelepathyQt/Account>

#include <KIcon>

AccountsTreeProxyModel::AccountsTreeProxyModel(QAbstractItemModel *sourceModel, const Tp::AccountManagerPtr &accountManager) :
    AbstractGroupingProxyModel(sourceModel),
    m_accountManager(accountManager)
{

}

QSet<QString> AccountsTreeProxyModel::groupsForIndex(const QModelIndex &sourceIndex) const
{
    const Tp::AccountPtr account = sourceIndex.data(ContactsModel::AccountRole).value<Tp::AccountPtr>();
    if (account) {
        qDebug() << "account";
        return QSet<QString>() << account->objectPath();
    } else {
        qDebug() << "no account";
        return QSet<QString>() << QLatin1String("Unknown");
    }
}


QVariant AccountsTreeProxyModel::dataForGroup(const QString &group, int role) const
{
    Tp::AccountPtr account;
    switch(role) {
    case Qt::DisplayRole:
        account = m_accountManager->accountForObjectPath(group);
        if (account) {
            return account->normalizedName();
        }
        break;
    case ContactsModel::IconRole:
        account = m_accountManager->accountForObjectPath(group);
        if (account) {
            return account->iconName();
        }
        break;
    case ContactsModel::AccountRole:
        return QVariant::fromValue(m_accountManager->accountForObjectPath(group));
    case ContactsModel::TypeRole:
        return ContactsModel::AccountRowType;
    case ContactsModel::EnabledRole:
        account = m_accountManager->accountForObjectPath(group);
        if (account) {
            return account->isEnabled();
        }
        return true;
    case ContactsModel::ConnectionStatusRole:
        account = m_accountManager->accountForObjectPath(group);
        if (account) {
            return account->connectionStatus();
        }
        return Tp::ConnectionStatusConnected;
    }
    return QVariant();
}
