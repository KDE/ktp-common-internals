#include "groups-tree-proxy-model.h"

#include "contacts-model.h"

#include <TelepathyQt/Account>

#include <KLocalizedString>

GroupsTreeProxyModel::GroupsTreeProxyModel(QAbstractItemModel *sourceModel) :
    AbstractGroupingProxyModel(sourceModel)
{
}

QSet<QString> GroupsTreeProxyModel::groupsForIndex(const QModelIndex &sourceIndex) const
{
    QStringList groups = sourceIndex.data(ContactsModel::GroupsRole).value<QStringList>();
    if (groups.isEmpty()) {
        groups.append(QLatin1String("_unsorted"));
    }

    return groups.toSet();
}


QVariant GroupsTreeProxyModel::dataForGroup(const QString &group, int role) const
{
    switch (role) {
    case ContactsModel::TypeRole:
        return ContactsModel::GroupRowType;
    case Qt::DisplayRole:
        if (group == QLatin1String("_unsorted")) {
            return i18n("Unsorted");
        } else {
            return group;
        }
    case ContactsModel::IdRole:
        return group;
    }
    return QVariant();
}
