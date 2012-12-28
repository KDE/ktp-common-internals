#ifndef ACCOUNTSTREEPROXYMODEL_H
#define ACCOUNTSTREEPROXYMODEL_H

#include "abstract-grouping-proxy-model.h"

#include <TelepathyQt/AccountManager>

#include <KTp/ktp-export.h>

class KTP_EXPORT AccountsTreeProxyModel : public AbstractGroupingProxyModel
{
    Q_OBJECT
public:
    AccountsTreeProxyModel(QAbstractItemModel *sourceModel, const Tp::AccountManagerPtr &accountManager);
   
    virtual QSet<QString> groupsForIndex(const QModelIndex &sourceIndex) const;
    virtual QVariant dataForGroup(const QString &group, int role) const;
private:
    Tp::AccountManagerPtr m_accountManager;
};

#endif // ACCOUNTSTREEPROXYMODEL_H
