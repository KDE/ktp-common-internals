/*
 * Provide some filters on the account model
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2011 Martin Klapetek <martin dot klapetek at gmail dot com>
 * Copyright (C) 2012 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
 * Copyright (C) 2012 Dominik Cermak <d.cermak@arcor.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "contacts-filter-model.h"

#include "types.h"

#include <presence.h>

#include <KDebug>


class KTp::ContactsFilterModel::Private
{
public:
    Private(ContactsFilterModel *parent)
        : q(parent),
          presenceTypeFilterFlags(DoNotFilterByCapability),
          capabilityFilterFlags(DoNotFilterByCapability),
          subscriptionStateFilterFlags(DoNotFilterBySubscription),
          globalFilterMatchFlags(Qt::MatchContains),
          displayNameFilterMatchFlags(Qt::MatchContains),
          nicknameFilterMatchFlags(Qt::MatchContains),
          aliasFilterMatchFlags(Qt::MatchContains),
          groupsFilterMatchFlags(Qt::MatchContains),
          idFilterMatchFlags(Qt::MatchContains)
    {
    }

    ContactsFilterModel *q;

    PresenceTypeFilterFlags presenceTypeFilterFlags;
    CapabilityFilterFlags capabilityFilterFlags;
    SubscriptionStateFilterFlags subscriptionStateFilterFlags;

    QString globalFilterString;
    Qt::MatchFlags globalFilterMatchFlags;

    QString displayNameFilterString;
    QString nicknameFilterString;
    QString aliasFilterString;
    QString groupsFilterString;
    QString idFilterString;
    Qt::MatchFlags displayNameFilterMatchFlags;
    Qt::MatchFlags nicknameFilterMatchFlags;
    Qt::MatchFlags aliasFilterMatchFlags;
    Qt::MatchFlags groupsFilterMatchFlags;
    Qt::MatchFlags idFilterMatchFlags;

    bool filterAcceptsAccount(const QModelIndex &index) const;
    bool filterAcceptsContact(const QModelIndex &index) const;
    bool filterAcceptsGroup(const QModelIndex &index);

    void countContacts(const QModelIndex &sourceParent);
    void sourceModelChanged(const QModelIndex &sourceIndex);

    QHash<QString, int> m_onlineContactsCounts;
    QHash<QString, int> m_totalContactsCounts;
};

using namespace KTp;

bool ContactsFilterModel::Private::filterAcceptsAccount(const QModelIndex &index) const
{
    // Check capability
    if (capabilityFilterFlags != DoNotFilterByCapability) {
        if ((capabilityFilterFlags & FilterByTextChatCapability)
                && !index.data(KTp::ContactCanTextChatRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByAudioCallCapability)
                && !index.data(KTp::ContactCanAudioCallRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByVideoCallCapability)
                && !index.data(KTp::ContactCanVideoCallRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByFileTransferCapability)
                && !index.data(KTp::ContactCanFileTransferRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByDesktopSharingCapability)
                && !index.data(ContactsModel::DesktopSharingCapabilityRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterBySSHContactCapability)
                && !index.data(ContactsModel::SSHContactCapabilityRole).toBool()) {
            return false;
        }
    }

    return true;
}

bool ContactsFilterModel::Private::filterAcceptsContact(const QModelIndex &index) const
{
    // Presence type, capability and subscription state are always checked
    // Then if global filter is set we can return true if a result is found for
    // any of the strings, otherwise we check all of them

    Q_ASSERT(index.isValid());
    if (!index.isValid()) {
        return false;
    }

    // Check presence type
    if (presenceTypeFilterFlags != DoNotFilterByPresence) {
        switch (static_cast<Tp::ConnectionPresenceType>(index.data(KTp::ContactPresenceTypeRole).toUInt())) {
        case Tp::ConnectionPresenceTypeUnset:
            if (presenceTypeFilterFlags & HidePresenceTypeUnset) {
                return false;
            }
            break;
        case Tp::ConnectionPresenceTypeOffline:
            if (presenceTypeFilterFlags & HidePresenceTypeOffline) {
                return false;
            }
            break;
        case Tp::ConnectionPresenceTypeAvailable:
            if (presenceTypeFilterFlags & HidePresenceTypeAvailable) {
                return false;
            }
            break;
        case Tp::ConnectionPresenceTypeAway:
            if (presenceTypeFilterFlags & HidePresenceTypeAway) {
                return false;
            }
            break;
        case Tp::ConnectionPresenceTypeExtendedAway:
            if (presenceTypeFilterFlags & HidePresenceTypeExtendedAway) {
                return false;
            }
            break;
        case Tp::ConnectionPresenceTypeHidden:
            if (presenceTypeFilterFlags & HidePresenceTypeHidden) {
                return false;
            }
            break;
        case Tp::ConnectionPresenceTypeBusy:
            if (presenceTypeFilterFlags & HidePresenceTypeBusy) {
                return false;
            }
            break;
        case Tp::ConnectionPresenceTypeUnknown:
            if (presenceTypeFilterFlags & HidePresenceTypeUnknown) {
                return false;
            }
            break;
        case Tp::ConnectionPresenceTypeError:
            if (presenceTypeFilterFlags & HidePresenceTypeError) {
                return false;
            }
            break;
        default:
            //This should never happen
            Q_ASSERT(false);
            return false;
        }
    }


    // Check capability
    if (capabilityFilterFlags != DoNotFilterByCapability) {
        if ((capabilityFilterFlags & FilterByTextChatCapability)
                && !index.data(KTp::ContactCanTextChatRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByAudioCallCapability)
                && !index.data(KTp::ContactCanAudioCallRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByVideoCallCapability)
                && !index.data(KTp::ContactCanVideoCallRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByFileTransferCapability)
                && !index.data(KTp::ContactCanFileTransferRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByDesktopSharingCapability)
                && !index.data(ContactsModel::DesktopSharingCapabilityRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterBySSHContactCapability)
                && !index.data(ContactsModel::SSHContactCapabilityRole).toBool()) {
            return false;
        }
    }


    // Check subscription state
    if (subscriptionStateFilterFlags != DoNotFilterBySubscription) {
        switch (index.data(KTp::ContactSubscriptionStateRole).toUInt()) {
        case Tp::Contact::PresenceStateNo:
            if (subscriptionStateFilterFlags & HideSubscriptionStateNo) {
                return false;
            }
            break;
        case Tp::Contact::PresenceStateAsk:
            if (subscriptionStateFilterFlags & HideSubscriptionStateAsk) {
                return false;
            }
            break;
        case Tp::Contact::PresenceStateYes:
            if (subscriptionStateFilterFlags & HideSubscriptionStateYes) {
                return false;
            }
            break;
        default:
            //This should never happen
            Q_ASSERT(false);
            return false;
        }

        switch (index.data(KTp::ContactPublishStateRole).toUInt()) {
        case Tp::Contact::PresenceStateNo:
            if (subscriptionStateFilterFlags & HidePublishStateNo) {
                return false;
            }
            break;
        case Tp::Contact::PresenceStateAsk:
            if (subscriptionStateFilterFlags & HidePublishStateAsk) {
                return false;
            }
            break;
        case Tp::Contact::PresenceStateYes:
            if (subscriptionStateFilterFlags & HidePublishStateYes) {
                return false;
            }
            break;
        default:
            //This should never happen
            Q_ASSERT(false);
            return false;
        }

        if (index.data(KTp::ContactIsBlockedRole).toBool()) {
            if (subscriptionStateFilterFlags & HideBlocked) {
                return false;
            }
        } else {
            if (subscriptionStateFilterFlags & HideNonBlocked) {
                return false;
            }
        }
    }


    if (!globalFilterString.isEmpty()) {
        // Check global filter (search on all the roles)

        // Check display name
        if (!q->match(index, Qt::DisplayRole, globalFilterString, 1, globalFilterMatchFlags).isEmpty()) {
            return true;
        }

        // Check nickname
        if (!q->match(index, ContactsModel::NicknameRole, globalFilterString, 1, globalFilterMatchFlags).isEmpty()) {
            return true;
        }

        // check alias
        if (!q->match(index, ContactsModel::AliasRole, globalFilterString, 1, globalFilterMatchFlags).isEmpty()) {
            return true;
        }

        // check groups
        // TODO Check if exact match on a single group works
        if (!q->match(index, KTp::ContactGroupsRole, globalFilterString, 1, globalFilterMatchFlags).isEmpty()) {
            return true;
        }

        // Check id
        if (!q->match(index, KTp::IdRole, globalFilterString, 1, globalFilterMatchFlags).isEmpty()) {
            return true;
        }

        return false;
    } else {
        // Check on single filters
        // Check display name
        if (!displayNameFilterString.isEmpty()) {
            if (q->match(index, Qt::DisplayRole, displayNameFilterString, 1, displayNameFilterMatchFlags).isEmpty()) {
                return false;
            }
        }

        // Check nickname
        if (!nicknameFilterString.isEmpty()) {
            if (q->match(index, ContactsModel::NicknameRole, nicknameFilterString, 1, nicknameFilterMatchFlags).isEmpty()) {
                return false;
            }
        }

        // check alias
        if (!aliasFilterString.isEmpty()) {
            if (q->match(index, ContactsModel::AliasRole, aliasFilterString, 1, aliasFilterMatchFlags).isEmpty()) {
                return false;
            }
        }
        // check groups
        // TODO Check if exact match on a single group works
        if (!groupsFilterString.isEmpty()) {
            if (q->match(index, KTp::ContactGroupsRole, groupsFilterString, 1, groupsFilterMatchFlags).isEmpty()) {
                return false;
            }
        }

        // Check id
        if (!idFilterString.isEmpty()) {
            if (q->match(index, KTp::IdRole, idFilterString, 1, idFilterMatchFlags).isEmpty()) {
                return false;
            }
        }
    }

    return true;
}

bool ContactsFilterModel::Private::filterAcceptsGroup(const QModelIndex &index)
{
    QString groupName = index.data(KTp::IdRole).toString();

    if (presenceTypeFilterFlags != DoNotFilterByPresence) {
        // If there is no cached value, create one
        if (!m_onlineContactsCounts.contains(groupName)) {
            countContacts(index);
        }

        // Don't accept groups with no online contacts
        if (m_onlineContactsCounts.value(groupName) == 0) {
//             return false;
        }
    }
    else {
        // If there is no cached value, create one
        if (!m_totalContactsCounts.contains(groupName)) {
            countContacts(index);
        }

        // Don't accept groups with no total contacts
        if (m_totalContactsCounts.value(groupName) == 0) {
//             return false;
        }
    }
    return true;
}

void ContactsFilterModel::Private::countContacts(const QModelIndex &sourceParent)
{
    QString key;

    KTp::RowType rowType = static_cast<KTp::RowType>(sourceParent.data(KTp::RowTypeRole).toInt());
    if (rowType == KTp::GroupRowType) {
        key = sourceParent.data(Qt::DisplayRole).toString();
    } else if (rowType == KTp::AccountRowType) {
        Tp::AccountPtr account = sourceParent.data(KTp::AccountRole).value<Tp::AccountPtr>();
        if (account.isNull()) {
            return;
        }
        key = account->uniqueIdentifier();
    } else {
        return;
    }

    // Count the online contacts
    int tmpCounter = 0;

    for (int i = 0; i < q->sourceModel()->rowCount(sourceParent); ++i) {
        QModelIndex child = q->sourceModel()->index(i, 0, sourceParent);

        // We want all online contacts that are accepted by the filter
        if (q->filterAcceptsRow(child.row(), sourceParent)
            && child.data(KTp::ContactPresenceTypeRole).toUInt() != Tp::ConnectionPresenceTypeOffline
            && child.data(KTp::ContactPresenceTypeRole).toUInt() != Tp::ConnectionPresenceTypeUnknown) {
            tmpCounter++;
        }
    }

    m_onlineContactsCounts.insert(key, tmpCounter);

    // Now count the total contacts accepted by the filter (but ignore presence filter).
    // Save the presenceTypeFilterFlags to reapply them later, because we need to disable
    // presence filtering to get the right numbers
    PresenceTypeFilterFlags saved = q->presenceTypeFilterFlags();
    presenceTypeFilterFlags = ContactsFilterModel::DoNotFilterByPresence;

    tmpCounter = 0;
    for (int i = 0; i < q->sourceModel()->rowCount(sourceParent); ++i) {
        QModelIndex child = q->sourceModel()->index(i, 0, sourceParent);
        if (q->filterAcceptsRow(child.row(), sourceParent)) {
            tmpCounter++;
        }
    }

    // Restore the saved presenceTypeFilterFlags
    presenceTypeFilterFlags = saved;

    m_totalContactsCounts.insert(key, tmpCounter);
}

void ContactsFilterModel::Private::sourceModelChanged(const QModelIndex &sourceIndex)
{
    QModelIndex groupSourceIndex;
    if (sourceIndex.data(KTp::RowTypeRole).toUInt() == KTp::ContactRowType) {
        groupSourceIndex = sourceIndex.parent();
    } else {
        groupSourceIndex = sourceIndex;
    }

    countContacts(groupSourceIndex);

    const QModelIndex mappedIndex = q->mapFromSource(sourceIndex);
    Q_EMIT q->dataChanged(mappedIndex, mappedIndex);
}


ContactsFilterModel::ContactsFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent),
      d(new Private(this))
{
}

ContactsFilterModel::~ContactsFilterModel()
{
    delete d;
}

QVariant ContactsFilterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    QModelIndex sourceIndex = mapToSource(index);
    if (!sourceIndex.isValid()) {
        return QVariant();
    }

    // Special handling for the counts
    KTp::RowType rowType = static_cast<KTp::RowType>(sourceIndex.data(KTp::RowTypeRole).toInt());
    if (role == KTp::HeaderOnlineUsersRole) {
        if (rowType == KTp::GroupRowType) {
            const QString groupName = sourceIndex.data(Qt::DisplayRole).toString();
            // If there is no cached value, create one
            if (!d->m_onlineContactsCounts.contains(groupName)) {
                d->countContacts(sourceIndex);
            }
            return d->m_onlineContactsCounts.value(groupName);
        } else if (rowType == KTp::AccountRowType) {
            const Tp::AccountPtr account = sourceIndex.data(KTp::AccountRole).value<Tp::AccountPtr>();
            if (account.isNull()) {
                return 0;
            }
            // If there is no cached value, create one
            if (!d->m_onlineContactsCounts.contains(account->uniqueIdentifier())) {
                d->countContacts(sourceIndex);
            }
            return d->m_onlineContactsCounts.value(account->uniqueIdentifier());
        }
    } else if (role == KTp::HeaderTotalUsersRole) {
        if (rowType == KTp::GroupRowType) {
            const QString groupName = sourceIndex.data(Qt::DisplayRole).toString();
            // If there is no cached value, create one
            if (!d->m_totalContactsCounts.contains(groupName)) {
                d->countContacts(sourceIndex);
            }
            return d->m_totalContactsCounts.value(groupName);
        } else if (rowType == KTp::AccountRowType) {
            const Tp::AccountPtr account = sourceIndex.data(KTp::AccountRole).value<Tp::AccountPtr>();
            if (account.isNull()) {
                return 0;
            }
            // If there is no cached value, create one
            if (!d->m_totalContactsCounts.contains(account->uniqueIdentifier())) {
                d->countContacts(sourceIndex);
            }
            return d->m_totalContactsCounts.value(account->uniqueIdentifier());
        }
    }

    // In all other cases just delegate it to the source model
    return sourceModel()->data(sourceIndex, role);
}

void ContactsFilterModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    // Disconnect the previous source model
    if (this->sourceModel()) {
        disconnect(this->sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(sourceModelChanged(QModelIndex)));
        disconnect(this->sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(sourceModelChanged(QModelIndex)));
        disconnect(this->sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(sourceModelChanged(QModelIndex)));
        disconnect(this->sourceModel(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
                this, SLOT(sourceModelChanged(QModelIndex)));
    }

    // Clear all cached values as they aren't valid anymore because the source model changed.
    d->m_onlineContactsCounts.clear();
    d->m_totalContactsCounts.clear();
    QSortFilterProxyModel::setSourceModel(sourceModel);

    // Connect the new source model
    connect(this->sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(sourceModelChanged(QModelIndex)));
    connect(this->sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(sourceModelChanged(QModelIndex)));
    connect(this->sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(sourceModelChanged(QModelIndex)));
    connect(this->sourceModel(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(sourceModelChanged(QModelIndex)));
}

void ContactsFilterModel::invalidateFilter()
{
    // Clear all cached values as they aren't valid anymore because the filter changed.
    d->m_onlineContactsCounts.clear();
    d->m_totalContactsCounts.clear();
    QSortFilterProxyModel::invalidateFilter();
}

ContactsFilterModel::PresenceTypeFilterFlags ContactsFilterModel::presenceTypeFilterFlags() const
{
    return d->presenceTypeFilterFlags;
}

void ContactsFilterModel::clearPresenceTypeFilterFlags()
{
    setPresenceTypeFilterFlags(DoNotFilterByPresence);
}

void ContactsFilterModel::setPresenceTypeFilterFlags(ContactsFilterModel::PresenceTypeFilterFlags presenceTypeFilterFlags)
{
    if (d->presenceTypeFilterFlags != presenceTypeFilterFlags) {
        d->presenceTypeFilterFlags = presenceTypeFilterFlags;
        invalidateFilter();
        Q_EMIT presenceTypeFilterFlagsChanged(presenceTypeFilterFlags);
    }
}

ContactsFilterModel::CapabilityFilterFlags ContactsFilterModel::capabilityFilterFlags() const
{
    return d->capabilityFilterFlags;
}

void ContactsFilterModel::clearCapabilityFilterFlags()
{
    setCapabilityFilterFlags(DoNotFilterByCapability);
}

void ContactsFilterModel::setCapabilityFilterFlags(ContactsFilterModel::CapabilityFilterFlags capabilityFilterFlags)
{
    if (d->capabilityFilterFlags != capabilityFilterFlags) {
        d->capabilityFilterFlags = capabilityFilterFlags;
        invalidateFilter();
        Q_EMIT capabilityFilterFlagsChanged(capabilityFilterFlags);
    }
}

ContactsFilterModel::SubscriptionStateFilterFlags ContactsFilterModel::subscriptionStateFilterFlags() const
{
    return d->subscriptionStateFilterFlags;
}

void ContactsFilterModel::clearSubscriptionStateFilterFlags()
{
    setSubscriptionStateFilterFlags(DoNotFilterBySubscription);
}

void ContactsFilterModel::setSubscriptionStateFilterFlags(ContactsFilterModel::SubscriptionStateFilterFlags subscriptionStateFilterFlags)
{
    if (d->subscriptionStateFilterFlags != subscriptionStateFilterFlags) {
        d->subscriptionStateFilterFlags = subscriptionStateFilterFlags;
        invalidateFilter();
        Q_EMIT subscriptionStateFilterFlagsChanged(subscriptionStateFilterFlags);
    }
}

QString ContactsFilterModel::globalFilterString() const
{
    return d->globalFilterString;
}

void ContactsFilterModel::clearGlobalFilterString()
{
    setGlobalFilterString(QString());
}

void ContactsFilterModel::setGlobalFilterString(const QString &globalFilterString)
{
    if (d->globalFilterString != globalFilterString) {
        d->globalFilterString = globalFilterString;
        invalidateFilter();
        Q_EMIT globalFilterStringChanged(globalFilterString);
    }
}

Qt::MatchFlags ContactsFilterModel::globalFilterMatchFlags() const
{
    return d->globalFilterMatchFlags;
}

void ContactsFilterModel::resetGlobalFilterMatchFlags()
{
    setGlobalFilterMatchFlags(Qt::MatchStartsWith | Qt::MatchWrap);
}

void ContactsFilterModel::setGlobalFilterMatchFlags(Qt::MatchFlags globalFilterMatchFlags)
{
    if (d->globalFilterMatchFlags != globalFilterMatchFlags) {
        d->globalFilterMatchFlags = globalFilterMatchFlags;
        invalidateFilter();
        Q_EMIT globalFilterMatchFlagsChanged(globalFilterMatchFlags);
    }
}

QString ContactsFilterModel::displayNameFilterString() const
{
    return d->displayNameFilterString;
}

void ContactsFilterModel::clearDisplayNameFilterString()
{
    setDisplayNameFilterString(QString());
}

void ContactsFilterModel::setDisplayNameFilterString(const QString &displayNameFilterString)
{
    if (d->displayNameFilterString != displayNameFilterString) {
        d->displayNameFilterString = displayNameFilterString;
        invalidateFilter();
        Q_EMIT displayNameFilterStringChanged(displayNameFilterString);
    }
}

Qt::MatchFlags ContactsFilterModel::displayNameFilterMatchFlags() const
{
    return d->displayNameFilterMatchFlags;
}

void ContactsFilterModel::resetDisplayNameFilterMatchFlags()
{
    setDisplayNameFilterMatchFlags(Qt::MatchStartsWith | Qt::MatchWrap);
}

void ContactsFilterModel::setDisplayNameFilterMatchFlags(Qt::MatchFlags displayNameFilterMatchFlags)
{
    if (d->displayNameFilterMatchFlags != displayNameFilterMatchFlags) {
        d->displayNameFilterMatchFlags = displayNameFilterMatchFlags;
        invalidateFilter();
        Q_EMIT displayNameFilterMatchFlagsChanged(displayNameFilterMatchFlags);
    }
}

QString ContactsFilterModel::nicknameFilterString() const
{
    return d->nicknameFilterString;
}

void ContactsFilterModel::clearNicknameFilterString()
{
    setNicknameFilterString(QString());
}

void ContactsFilterModel::setNicknameFilterString(const QString &nicknameFilterString)
{
    if (d->nicknameFilterString != nicknameFilterString) {
        d->nicknameFilterString = nicknameFilterString;
        invalidateFilter();
        Q_EMIT nicknameFilterStringChanged(nicknameFilterString);
    }
}

Qt::MatchFlags ContactsFilterModel::nicknameFilterMatchFlags() const
{
    return d->nicknameFilterMatchFlags;
}

void ContactsFilterModel::resetNicknameFilterMatchFlags()
{
    setNicknameFilterMatchFlags(Qt::MatchStartsWith | Qt::MatchWrap);
}

void ContactsFilterModel::setNicknameFilterMatchFlags(Qt::MatchFlags nicknameFilterMatchFlags)
{
    if (d->nicknameFilterMatchFlags != nicknameFilterMatchFlags) {
        d->nicknameFilterMatchFlags = nicknameFilterMatchFlags;
        invalidateFilter();
        Q_EMIT nicknameFilterMatchFlagsChanged(nicknameFilterMatchFlags);
    }
}

QString ContactsFilterModel::aliasFilterString() const
{
    return d->aliasFilterString;
}

void ContactsFilterModel::clearAliasFilterString()
{
    setAliasFilterString(QString());
}

void ContactsFilterModel::setAliasFilterString(const QString &aliasFilterString)
{
    if (d->aliasFilterString != aliasFilterString) {
        d->aliasFilterString = aliasFilterString;
        invalidateFilter();
        Q_EMIT aliasFilterStringChanged(aliasFilterString);
    }
}

Qt::MatchFlags ContactsFilterModel::aliasFilterMatchFlags() const
{
    return d->aliasFilterMatchFlags;
}

void ContactsFilterModel::resetAliasFilterMatchFlags()
{
    setAliasFilterMatchFlags(Qt::MatchStartsWith | Qt::MatchWrap);
}

void ContactsFilterModel::setAliasFilterMatchFlags(Qt::MatchFlags aliasFilterMatchFlags)
{
    if (d->aliasFilterMatchFlags != aliasFilterMatchFlags) {
        d->aliasFilterMatchFlags = aliasFilterMatchFlags;
        invalidateFilter();
        Q_EMIT aliasFilterMatchFlagsChanged(aliasFilterMatchFlags);
    }
}

QString ContactsFilterModel::groupsFilterString() const
{
    return d->groupsFilterString;
}

void ContactsFilterModel::clearGroupsFilterString()
{
    setGroupsFilterString(QString());
}

void ContactsFilterModel::setGroupsFilterString(const QString &groupsFilterString)
{
    if (d->groupsFilterString != groupsFilterString) {
        d->groupsFilterString = groupsFilterString;
        invalidateFilter();
        Q_EMIT groupsFilterStringChanged(groupsFilterString);
    }
}

Qt::MatchFlags ContactsFilterModel::groupsFilterMatchFlags() const
{
    return d->groupsFilterMatchFlags;
}

void ContactsFilterModel::resetGroupsFilterMatchFlags()
{
    setGroupsFilterMatchFlags(Qt::MatchStartsWith | Qt::MatchWrap);
}

void ContactsFilterModel::setGroupsFilterMatchFlags(Qt::MatchFlags groupsFilterMatchFlags)
{
    if (d->groupsFilterMatchFlags != groupsFilterMatchFlags) {
        d->groupsFilterMatchFlags = groupsFilterMatchFlags;
        invalidateFilter();
        Q_EMIT groupsFilterMatchFlagsChanged(groupsFilterMatchFlags);
    }
}

QString ContactsFilterModel::idFilterString() const
{
    return d->idFilterString;
}

void ContactsFilterModel::clearIdFilterString()
{
    setIdFilterString(QString());
}

void ContactsFilterModel::setIdFilterString(const QString &idFilterString)
{
    if (d->idFilterString != idFilterString) {
        d->idFilterString = idFilterString;
        invalidateFilter();
        Q_EMIT idFilterStringChanged(idFilterString);
    }
}

Qt::MatchFlags ContactsFilterModel::idFilterMatchFlags() const
{
    return d->idFilterMatchFlags;
}

void ContactsFilterModel::resetIdFilterMatchFlags()
{
    setIdFilterMatchFlags(Qt::MatchStartsWith | Qt::MatchWrap);
}

void ContactsFilterModel::setIdFilterMatchFlags(Qt::MatchFlags idFilterMatchFlags)
{
    if (d->idFilterMatchFlags != idFilterMatchFlags) {
        d->idFilterMatchFlags = idFilterMatchFlags;
        invalidateFilter();
        Q_EMIT idFilterMatchFlagsChanged(idFilterMatchFlags);
    }
}

bool ContactsFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    int type = index.data(KTp::RowTypeRole).toInt();
    if (type == KTp::ContactRowType) {
        return d->filterAcceptsContact(index);
    }
    else if (type == KTp::AccountRowType) {
        return d->filterAcceptsAccount(index);
    }
    else if (type == KTp::GroupRowType) {
        return d->filterAcceptsGroup(index);
    }
    else {
        kDebug() << "Unknown type found in Account Filter";
        return true;
    }
}

bool ContactsFilterModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{

    QString leftDisplayedName = sourceModel()->data(left).toString();
    QString rightDisplayedName = sourceModel()->data(right).toString();

    switch (sortRole()) {
        case KTp::ContactPresenceTypeRole:
    {
        Tp::ConnectionPresenceType leftPresence = (Tp::ConnectionPresenceType)left.data(KTp::ContactPresenceTypeRole).toUInt();
        Tp::ConnectionPresenceType rightPresence = (Tp::ConnectionPresenceType)right.data(KTp::ContactPresenceTypeRole).toUInt();

        if (leftPresence == rightPresence) {
            //presences are the same, compare client types

            bool leftPhone = left.data(KTp::ContactClientTypesRole).toStringList().contains(QLatin1String("phone"));
            bool rightPhone = right.data(KTp::ContactClientTypesRole).toStringList().contains(QLatin1String("phone"));

            if (leftPhone && ! rightPhone) {
                return false;
            }
            else if (rightPhone && !leftPhone) {
                return true;
            }

            return QString::localeAwareCompare(leftDisplayedName, rightDisplayedName) < 0;
        } else {
            if (leftPresence == Tp::ConnectionPresenceTypeAvailable) {
                return true;
            }
            if (leftPresence == Tp::ConnectionPresenceTypeUnset ||
                    leftPresence == Tp::ConnectionPresenceTypeOffline ||
                    leftPresence == Tp::ConnectionPresenceTypeUnknown ||
                    leftPresence == Tp::ConnectionPresenceTypeError) {
                return false;
            }

            return KTp::Presence::sortPriority(leftPresence) < KTp::Presence::sortPriority(rightPresence);
        }
    }
    case Qt::DisplayRole:
    {
        QString leftAlias = sourceModel()->data(left, Qt::DisplayRole).toString();
        QString rightAlias = sourceModel()->data(right, Qt::DisplayRole).toString();
        return QString::localeAwareCompare(rightAlias, rightAlias) < 0;
    }
    default:
        return QString::localeAwareCompare(leftDisplayedName, rightDisplayedName) < 0;
    }
}


QModelIndexList ContactsFilterModel::match(const QModelIndex &start, int role,
                                          const QVariant &value, int hits,
                                          Qt::MatchFlags flags) const
{
    if (!start.isValid()) {
        return QModelIndexList();
    }

    QModelIndexList result;
    uint matchType = flags & 0x0F;
    Qt::CaseSensitivity cs = flags & Qt::MatchCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    bool recurse = flags & Qt::MatchRecursive;
    bool allHits = (hits == -1);
    QString text; // only convert to a string if it is needed
    QVariant v = start.data(role);
    // QVariant based matching
    if (matchType == Qt::MatchExactly) {
        if (value == v)
            result.append(start);
    } else { // QString based matching
        if (text.isEmpty()) // lazy conversion
            text = value.toString();
        QString t = v.toString();
        switch (matchType) {
        case Qt::MatchRegExp:
            if (QRegExp(text, cs).exactMatch(t))
                result.append(start);
            break;
        case Qt::MatchWildcard:
            if (QRegExp(text, cs, QRegExp::Wildcard).exactMatch(t))
                result.append(start);
            break;
        case Qt::MatchStartsWith:
            if (t.startsWith(text, cs))
                result.append(start);
            break;
        case Qt::MatchEndsWith:
            if (t.endsWith(text, cs))
                result.append(start);
            break;
        case Qt::MatchFixedString:
            if (t.compare(text, cs) == 0)
                result.append(start);
            break;
        case Qt::MatchContains:
        default:
            if (t.contains(text, cs))
                result.append(start);
        }
    }
    if (recurse && hasChildren(start)) { // search the hierarchy
        result += match(index(0, start.column(), start), role,
                        (text.isEmpty() ? value : text),
                        (allHits ? -1 : hits - result.count()), flags);
    }
    return result;
}


#include "contacts-filter-model.moc"
