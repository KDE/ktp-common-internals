#ifndef GROUPSTREEPROXYMODEL_H
#define GROUPSTREEPROXYMODEL_H

#include "abstract-grouping-proxy-model.h"

#include <TelepathyQt/AccountManager>

#include <KTp/ktp-export.h>

class KTP_EXPORT GroupsTreeProxyModel : public AbstractGroupingProxyModel
{
    Q_OBJECT
public:
    GroupsTreeProxyModel(QAbstractItemModel *sourceModel);
   
    virtual QSet<QString> groupsForIndex(const QModelIndex &sourceIndex) const;
    virtual QVariant dataForGroup(const QString &group, int role) const;
};

#endif // ACCOUNTSTREEPROXYMODEL_H
