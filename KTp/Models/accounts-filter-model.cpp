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

#include "accounts-filter-model.h"

#include "accounts-model.h"
#include "groups-model.h"
#include "groups-model-item.h"
#include "contact-model-item.h"
#include "accounts-model-item.h"
#include "proxy-tree-node.h"

#include <presence.h>

#include <KDebug>


class AccountsFilterModel::Private
{
public:
    Private(AccountsFilterModel *parent)
        : q(parent),
          presenceTypeFilterFlags(DoNotFilterByCapability),
          capabilityFilterFlags(DoNotFilterByCapability),
          subscriptionStateFilterFlags(DoNotFilterBySubscription),
          globalFilterMatchFlags(Qt::MatchContains),
          displayNameFilterMatchFlags(Qt::MatchContains),
          nicknameFilterMatchFlags(Qt::MatchContains),
          aliasFilterMatchFlags(Qt::MatchContains),
          groupsFilterMatchFlags(Qt::MatchContains)
    {
    }

    AccountsFilterModel *q;

    PresenceTypeFilterFlags presenceTypeFilterFlags;
    CapabilityFilterFlags capabilityFilterFlags;
    SubscriptionStateFilterFlags subscriptionStateFilterFlags;

    QString globalFilterString;
    Qt::MatchFlags globalFilterMatchFlags;

    QString displayNameFilterString;
    QString nicknameFilterString;
    QString aliasFilterString;
    QString groupsFilterString;
    Qt::MatchFlags displayNameFilterMatchFlags;
    Qt::MatchFlags nicknameFilterMatchFlags;
    Qt::MatchFlags aliasFilterMatchFlags;
    Qt::MatchFlags groupsFilterMatchFlags;

    bool filterAcceptsAccount(const QModelIndex &index) const;
    bool filterAcceptsContact(const QModelIndex &index) const;
    bool filterAcceptsGroup(const QModelIndex &index) const;

    QHash<QString, int> m_onlineContactsCounts;
    QHash<QString, int> m_totalContactsCounts;
};

bool AccountsFilterModel::Private::filterAcceptsAccount(const QModelIndex &index) const
{
    // Hide disabled accounts
    if (!index.data(AccountsModel::EnabledRole).toBool()) {
        return false;
    }

    // Hide disconnected accounts
    if (index.data(AccountsModel::ConnectionStatusRole).toUInt()
        != Tp::ConnectionStatusConnected) {
        return false;
    }

    // Check capability
    if (capabilityFilterFlags != DoNotFilterByCapability) {
        if ((capabilityFilterFlags & FilterByTextChatCapability)
                && !index.data(AccountsModel::TextChatCapabilityRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByMediaCallCapability)
                && !index.data(AccountsModel::MediaCallCapabilityRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByAudioCallCapability)
                && !index.data(AccountsModel::AudioCallCapabilityRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByVideoCallCapability)
                && !index.data(AccountsModel::VideoCallCapabilityRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByFileTransferCapability)
                && !index.data(AccountsModel::FileTransferCapabilityRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByDesktopSharingCapability)
                && !index.data(AccountsModel::DesktopSharingCapabilityRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterBySSHContactCapability)
                && !index.data(AccountsModel::SSHContactCapabilityRole).toBool()) {
            return false;
        }
    }

    return true;
}

bool AccountsFilterModel::Private::filterAcceptsContact(const QModelIndex &index) const
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
        switch (static_cast<Tp::ConnectionPresenceType>(index.data(AccountsModel::PresenceTypeRole).toUInt())) {
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
                && !index.data(AccountsModel::TextChatCapabilityRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByMediaCallCapability)
                && !index.data(AccountsModel::MediaCallCapabilityRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByAudioCallCapability)
                && !index.data(AccountsModel::AudioCallCapabilityRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByVideoCallCapability)
                && !index.data(AccountsModel::VideoCallCapabilityRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByFileTransferCapability)
                && !index.data(AccountsModel::FileTransferCapabilityRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterByDesktopSharingCapability)
                && !index.data(AccountsModel::DesktopSharingCapabilityRole).toBool()) {
            return false;
        }
        if ((capabilityFilterFlags & FilterBySSHContactCapability)
                && !index.data(AccountsModel::SSHContactCapabilityRole).toBool()) {
            return false;
        }
    }


    // Check subscription state
    if (subscriptionStateFilterFlags != DoNotFilterBySubscription) {
        switch (index.data(AccountsModel::SubscriptionStateRole).toUInt()) {
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

        switch (index.data(AccountsModel::PublishStateRole).toUInt()) {
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

        if (index.data(AccountsModel::BlockedRole).toBool()) {
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
        if (!q->match(index, AccountsModel::NicknameRole, globalFilterString, 1, globalFilterMatchFlags).isEmpty()) {
            return true;
        }

        // check alias
        if (!q->match(index, AccountsModel::AliasRole, globalFilterString, 1, globalFilterMatchFlags).isEmpty()) {
            return true;
        }

        // check groups
        // TODO Check if exact match on a single group works
        if (!q->match(index, AccountsModel::GroupsRole, globalFilterString, 1, globalFilterMatchFlags).isEmpty()) {
            return true;
        }
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
            if (q->match(index, AccountsModel::NicknameRole, nicknameFilterString, 1, nicknameFilterMatchFlags).isEmpty()) {
                return false;
            }
        }

        // check alias
        if (!aliasFilterString.isEmpty()) {
            if (q->match(index, AccountsModel::AliasRole, aliasFilterString, 1, aliasFilterMatchFlags).isEmpty()) {
                return false;
            }
        }
        // check groups
        // TODO Check if exact match on a single group works
        if (!groupsFilterString.isEmpty()) {
            if (q->match(index, AccountsModel::GroupsRole, groupsFilterString, 1, groupsFilterMatchFlags).isEmpty()) {
                return false;
            }
        }
    }

    return true;
}

bool AccountsFilterModel::Private::filterAcceptsGroup(const QModelIndex &index) const
{
    QVariant item = index.data(AccountsModel::ItemRole);
    GroupsModelItem *gmItem = item.value<GroupsModelItem*>();

    if (presenceTypeFilterFlags != DoNotFilterByPresence) {
        // If there is no cached value, create one
        if (!m_onlineContactsCounts.contains(gmItem->groupName())) {
            q->countContacts(index);
        }

        // Don't accept groups with no online contacts
        if (m_onlineContactsCounts.value(gmItem->groupName()) == 0) {
            return false;
        }
    }
    else {
        // If there is no cached value, create one
        if (!m_totalContactsCounts.contains(gmItem->groupName())) {
            q->countContacts(index);
        }

        // Don't accept groups with no total contacts
        if (m_totalContactsCounts.value(gmItem->groupName()) == 0) {
            return false;
        }
    }
    return true;
}

AccountsFilterModel::AccountsFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent),
      d(new Private(this))
{
}

AccountsFilterModel::~AccountsFilterModel()
{
    delete d;
}

QVariant AccountsFilterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    QModelIndex sourceIndex = mapToSource(index);
    if (!sourceIndex.isValid()) {
        return QVariant();
    }

    // Special handling for the counts
    if (role == AccountsModel::OnlineUsersCountRole) {
        QVariant item = sourceIndex.data(AccountsModel::ItemRole);
        if (item.canConvert<GroupsModelItem*>()) {
            GroupsModelItem *gmItem = item.value<GroupsModelItem*>();
            // If there is no cached value, create one
            if (!d->m_onlineContactsCounts.contains(gmItem->groupName())) {
                countContacts(sourceIndex);
            }
            return d->m_onlineContactsCounts.value(gmItem->groupName());
        } else if (item.canConvert<AccountsModelItem*>()) {
            AccountsModelItem *amItem = item.value<AccountsModelItem*>();
            // If there is no cached value, create one
            if (!d->m_onlineContactsCounts.contains(amItem->data(AccountsModel::IdRole).toString())) {
                countContacts(sourceIndex);
            }
            return d->m_onlineContactsCounts.value(amItem->data(AccountsModel::IdRole).toString());
        }
    } else if (role == AccountsModel::TotalUsersCountRole) {
        QVariant item = sourceIndex.data(AccountsModel::ItemRole);
        if (item.canConvert<GroupsModelItem*>()) {
            GroupsModelItem *gmItem = item.value<GroupsModelItem*>();
            // If there is no cached value, create one
            if (!d->m_totalContactsCounts.contains(gmItem->groupName())) {
                countContacts(sourceIndex);
            }
            return d->m_totalContactsCounts.value(gmItem->groupName());
        } else if (item.canConvert<AccountsModelItem*>()) {
            AccountsModelItem *amItem = item.value<AccountsModelItem*>();
            // If there is no cached value, create one
            if (!d->m_totalContactsCounts.contains(amItem->data(AccountsModel::IdRole).toString())) {
                countContacts(sourceIndex);
            }
            return d->m_totalContactsCounts.value(amItem->data(AccountsModel::IdRole).toString());
        }
    }

    // In all other cases just delegate it to the source model
    return sourceModel()->data(mapToSource(index), role);
}

void AccountsFilterModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    // Disconnect the previous source model
    disconnect(this->sourceModel(),
               SIGNAL(dataChanged(QModelIndex,QModelIndex)),
               this,
               SLOT(countContacts(QModelIndex)));

    // Clear all cached values as they aren't valid anymore because the source model changed.
    d->m_onlineContactsCounts.clear();
    d->m_totalContactsCounts.clear();
    QSortFilterProxyModel::setSourceModel(sourceModel);

    // Connect the new source model
    connect(this->sourceModel(),
            SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this,
            SLOT(countContacts(QModelIndex)));
}

void AccountsFilterModel::invalidateFilter()
{
    // Clear all cached values as they aren't valid anymore because the filter changed.
    d->m_onlineContactsCounts.clear();
    d->m_totalContactsCounts.clear();
    QSortFilterProxyModel::invalidateFilter();
}

AccountsFilterModel::SortMode AccountsFilterModel::sortMode() const
{
    switch (sortRole()) {
    case Qt::DisplayRole:
        return DoNotSort;
    case AccountsModel::PresenceTypeRole:
        return SortByPresence;
    case AccountsModel::NicknameRole:
        return SortByNickname;
    case AccountsModel::AliasRole:
        return SortByAlias;
    default:
        //This should never happen
        Q_ASSERT(false);
        return DoNotSort;
    }
}

void AccountsFilterModel::resetSortMode()
{
    setSortMode(DoNotSort);
}

void AccountsFilterModel::setSortMode(SortMode sortMode)
{
    switch (sortMode) {
    case DoNotSort:
        setSortRole(Qt::DisplayRole);
        break;
    case SortByPresence:
        setSortRole(AccountsModel::PresenceTypeRole);
        break;
    case SortByNickname:
        setSortRole(AccountsModel::NicknameRole);
        break;
    case SortByAlias:
        setSortRole(AccountsModel::AliasRole);
        break;
    default:
        //This should never happen
        Q_ASSERT(false);
        return;
    }
    Q_EMIT sortModeChanged(sortMode);
}

AccountsFilterModel::PresenceTypeFilterFlags AccountsFilterModel::presenceTypeFilterFlags() const
{
    return d->presenceTypeFilterFlags;
}

void AccountsFilterModel::clearPresenceTypeFilterFlags()
{
    setPresenceTypeFilterFlags(DoNotFilterByPresence);
}

void AccountsFilterModel::setPresenceTypeFilterFlags(AccountsFilterModel::PresenceTypeFilterFlags presenceTypeFilterFlags)
{
    if (d->presenceTypeFilterFlags != presenceTypeFilterFlags) {
        d->presenceTypeFilterFlags = presenceTypeFilterFlags;
        invalidateFilter();
        Q_EMIT presenceTypeFilterFlagsChanged(presenceTypeFilterFlags);
    }
}

AccountsFilterModel::CapabilityFilterFlags AccountsFilterModel::capabilityFilterFlags() const
{
    return d->capabilityFilterFlags;
}

void AccountsFilterModel::clearCapabilityFilterFlags()
{
    setCapabilityFilterFlags(DoNotFilterByCapability);
}

void AccountsFilterModel::setCapabilityFilterFlags(AccountsFilterModel::CapabilityFilterFlags capabilityFilterFlags)
{
    if (d->capabilityFilterFlags != capabilityFilterFlags) {
        d->capabilityFilterFlags = capabilityFilterFlags;
        invalidateFilter();
        Q_EMIT capabilityFilterFlagsChanged(capabilityFilterFlags);
    }
}

AccountsFilterModel::SubscriptionStateFilterFlags AccountsFilterModel::subscriptionStateFilterFlags() const
{
    return d->subscriptionStateFilterFlags;
}

void AccountsFilterModel::clearSubscriptionStateFilterFlags()
{
    setSubscriptionStateFilterFlags(DoNotFilterBySubscription);
}

void AccountsFilterModel::setSubscriptionStateFilterFlags(AccountsFilterModel::SubscriptionStateFilterFlags subscriptionStateFilterFlags)
{
    if (d->subscriptionStateFilterFlags != subscriptionStateFilterFlags) {
        d->subscriptionStateFilterFlags = subscriptionStateFilterFlags;
        invalidateFilter();
        Q_EMIT subscriptionStateFilterFlagsChanged(subscriptionStateFilterFlags);
    }
}

QString AccountsFilterModel::globalFilterString() const
{
    return d->globalFilterString;
}

void AccountsFilterModel::clearGlobalFilterString()
{
    setGlobalFilterString(QString());
}

void AccountsFilterModel::setGlobalFilterString(const QString &globalFilterString)
{
    if (d->globalFilterString != globalFilterString) {
        d->globalFilterString = globalFilterString;
        invalidateFilter();
        Q_EMIT globalFilterStringChanged(globalFilterString);
    }
}

Qt::MatchFlags AccountsFilterModel::globalFilterMatchFlags() const
{
    return d->globalFilterMatchFlags;
}

void AccountsFilterModel::resetGlobalFilterMatchFlags()
{
    setGlobalFilterMatchFlags(Qt::MatchStartsWith | Qt::MatchWrap);
}

void AccountsFilterModel::setGlobalFilterMatchFlags(Qt::MatchFlags globalFilterMatchFlags)
{
    if (d->globalFilterMatchFlags != globalFilterMatchFlags) {
        d->globalFilterMatchFlags = globalFilterMatchFlags;
        invalidateFilter();
        Q_EMIT globalFilterMatchFlagsChanged(globalFilterMatchFlags);
    }
}

QString AccountsFilterModel::displayNameFilterString() const
{
    return d->displayNameFilterString;
}

void AccountsFilterModel::clearDisplayNameFilterString()
{
    setDisplayNameFilterString(QString());
}

void AccountsFilterModel::setDisplayNameFilterString(const QString &displayNameFilterString)
{
    if (d->displayNameFilterString != displayNameFilterString) {
        d->displayNameFilterString = displayNameFilterString;
        invalidateFilter();
        Q_EMIT displayNameFilterStringChanged(displayNameFilterString);
    }
}

Qt::MatchFlags AccountsFilterModel::displayNameFilterMatchFlags() const
{
    return d->displayNameFilterMatchFlags;
}

void AccountsFilterModel::resetDisplayNameFilterMatchFlags()
{
    setDisplayNameFilterMatchFlags(Qt::MatchStartsWith | Qt::MatchWrap);
}

void AccountsFilterModel::setDisplayNameFilterMatchFlags(Qt::MatchFlags displayNameFilterMatchFlags)
{
    if (d->displayNameFilterMatchFlags != displayNameFilterMatchFlags) {
        d->displayNameFilterMatchFlags = displayNameFilterMatchFlags;
        invalidateFilter();
        Q_EMIT displayNameFilterMatchFlagsChanged(displayNameFilterMatchFlags);
    }
}

QString AccountsFilterModel::nicknameFilterString() const
{
    return d->nicknameFilterString;
}

void AccountsFilterModel::clearNicknameFilterString()
{
    setNicknameFilterString(QString());
}

void AccountsFilterModel::setNicknameFilterString(const QString &nicknameFilterString)
{
    if (d->nicknameFilterString != nicknameFilterString) {
        d->nicknameFilterString = nicknameFilterString;
        invalidateFilter();
        Q_EMIT nicknameFilterStringChanged(nicknameFilterString);
    }
}

Qt::MatchFlags AccountsFilterModel::nicknameFilterMatchFlags() const
{
    return d->nicknameFilterMatchFlags;
}

void AccountsFilterModel::resetNicknameFilterMatchFlags()
{
    setNicknameFilterMatchFlags(Qt::MatchStartsWith | Qt::MatchWrap);
}

void AccountsFilterModel::setNicknameFilterMatchFlags(Qt::MatchFlags nicknameFilterMatchFlags)
{
    if (d->nicknameFilterMatchFlags != nicknameFilterMatchFlags) {
        d->nicknameFilterMatchFlags = nicknameFilterMatchFlags;
        invalidateFilter();
        Q_EMIT nicknameFilterMatchFlagsChanged(nicknameFilterMatchFlags);
    }
}

QString AccountsFilterModel::aliasFilterString() const
{
    return d->aliasFilterString;
}

void AccountsFilterModel::clearAliasFilterString()
{
    setAliasFilterString(QString());
}

void AccountsFilterModel::setAliasFilterString(const QString &aliasFilterString)
{
    if (d->aliasFilterString != aliasFilterString) {
        d->aliasFilterString = aliasFilterString;
        invalidateFilter();
        Q_EMIT aliasFilterStringChanged(aliasFilterString);
    }
}

Qt::MatchFlags AccountsFilterModel::aliasFilterMatchFlags() const
{
    return d->aliasFilterMatchFlags;
}

void AccountsFilterModel::resetAliasFilterMatchFlags()
{
    setAliasFilterMatchFlags(Qt::MatchStartsWith | Qt::MatchWrap);
}

void AccountsFilterModel::setAliasFilterMatchFlags(Qt::MatchFlags aliasFilterMatchFlags)
{
    if (d->aliasFilterMatchFlags != aliasFilterMatchFlags) {
        d->aliasFilterMatchFlags = aliasFilterMatchFlags;
        invalidateFilter();
        Q_EMIT aliasFilterMatchFlagsChanged(aliasFilterMatchFlags);
    }
}

QString AccountsFilterModel::groupsFilterString() const
{
    return d->groupsFilterString;
}

void AccountsFilterModel::clearGroupsFilterString()
{
    setGroupsFilterString(QString());
}

void AccountsFilterModel::setGroupsFilterString(const QString &groupsFilterString)
{
    if (d->groupsFilterString != groupsFilterString) {
        d->groupsFilterString = groupsFilterString;
        invalidateFilter();
        Q_EMIT groupsFilterStringChanged(groupsFilterString);
    }
}

Qt::MatchFlags AccountsFilterModel::groupsFilterMatchFlags() const
{
    return d->groupsFilterMatchFlags;
}

void AccountsFilterModel::resetGroupsFilterMatchFlags()
{
    setGroupsFilterMatchFlags(Qt::MatchStartsWith | Qt::MatchWrap);
}

void AccountsFilterModel::setGroupsFilterMatchFlags(Qt::MatchFlags groupsFilterMatchFlags)
{
    if (d->groupsFilterMatchFlags != groupsFilterMatchFlags) {
        d->groupsFilterMatchFlags = groupsFilterMatchFlags;
        invalidateFilter();
        Q_EMIT groupsFilterMatchFlagsChanged(groupsFilterMatchFlags);
    }
}

void AccountsFilterModel::countContacts(const QModelIndex &index) const
{
    QVariant item = index.data(AccountsModel::ItemRole);
    if (item.canConvert<GroupsModelItem*>()) {
        GroupsModelItem *gmItem = item.value<GroupsModelItem*>();

        // Count the online contacts
        int tmpCounter = 0;

        for (int i = 0; i < gmItem->size(); ++i) {
            ProxyTreeNode* proxyNode = qobject_cast<ProxyTreeNode*>(gmItem->childAt(i));
            Q_ASSERT(proxyNode);

            // We want all online contacts that are accepted by the filter
            if (filterAcceptsRow(gmItem->indexOf(gmItem->childAt(i)), index)
                && proxyNode->data(AccountsModel::PresenceTypeRole).toUInt() != Tp::ConnectionPresenceTypeOffline
                && proxyNode->data(AccountsModel::PresenceTypeRole).toUInt() != Tp::ConnectionPresenceTypeUnknown) {
                tmpCounter++;
            }
        }

        d->m_onlineContactsCounts.insert(gmItem->groupName(), tmpCounter);

        // Now count the total contacts accepted by the filter (but ignore presence filter).
        // Save the presenceTypeFilterFlags to reapply them later, because we need to disable
        // presence filtering to get the right numbers
        PresenceTypeFilterFlags saved = presenceTypeFilterFlags();
        d->presenceTypeFilterFlags = AccountsFilterModel::DoNotFilterByPresence;

        tmpCounter = 0;
        for (int i = 0; i < gmItem->size(); ++i) {
            if (filterAcceptsRow(gmItem->indexOf(gmItem->childAt(i)), index)) {
                tmpCounter++;
            }
        }

        // Restore the saved presenceTypeFilterFlags
        d->presenceTypeFilterFlags = saved;

        d->m_totalContactsCounts.insert(gmItem->groupName(), tmpCounter);
    } else if (item.canConvert<AccountsModelItem*>()) {
        AccountsModelItem *amItem = item.value<AccountsModelItem*>();

        // Count the online contacts
        int tmpCounter = 0;

        for (int i = 0; i < amItem->size(); ++i) {
            ContactModelItem* contactNode = qobject_cast<ContactModelItem*>(amItem->childAt(i));
            Q_ASSERT(contactNode);

            // We want all online contacts that are accepted by the filter
            if (filterAcceptsRow(amItem->indexOf(amItem->childAt(i)), index)
                && contactNode->data(AccountsModel::PresenceTypeRole).toUInt() != Tp::ConnectionPresenceTypeOffline
                && contactNode->data(AccountsModel::PresenceTypeRole).toUInt() != Tp::ConnectionPresenceTypeUnknown) {
                tmpCounter++;
            }
        }

        d->m_onlineContactsCounts.insert(amItem->data(AccountsModel::IdRole).toString(), tmpCounter);

        // Now count the total contacts accepted by the filter (but ignore presence filter).
        // Save the presenceTypeFilterFlags to reapply them later, because we need to disable
        // presence filtering to get the right numbers
        PresenceTypeFilterFlags saved = presenceTypeFilterFlags();
        d->presenceTypeFilterFlags = AccountsFilterModel::DoNotFilterByPresence;

        tmpCounter = 0;
        for (int i = 0; i < amItem->size(); ++i) {
            if (filterAcceptsRow(amItem->indexOf(amItem->childAt(i)), index)) {
                tmpCounter++;
            }
        }

        // Restore the saved presenceTypeFilterFlags
        d->presenceTypeFilterFlags = saved;

        d->m_totalContactsCounts.insert(amItem->data(AccountsModel::IdRole).toString(), tmpCounter);
    }
}

bool AccountsFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    int type = index.data(AccountsModel::ItemRole).userType();
    if (type == qMetaTypeId<ContactModelItem*>()) {
        return d->filterAcceptsContact(index);
    }
    else if (type == qMetaTypeId<AccountsModelItem*>()) {
        return d->filterAcceptsAccount(index);
    }
    else if (type == qMetaTypeId<GroupsModelItem*>()) {
        return d->filterAcceptsGroup(index);
    }
    else {
        kDebug() << "Unknown type found in Account Filter";
        return true;
    }
}

bool AccountsFilterModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{

    QString leftDisplayedName = sourceModel()->data(left).toString();
    QString rightDisplayedName = sourceModel()->data(right).toString();

    switch (sortRole()) {
    case AccountsModel::PresenceTypeRole:
    {
        Tp::ConnectionPresenceType leftPresence = (Tp::ConnectionPresenceType)sourceModel()->data(left, AccountsModel::PresenceTypeRole).toUInt();
        Tp::ConnectionPresenceType rightPresence = (Tp::ConnectionPresenceType)sourceModel()->data(right, AccountsModel::PresenceTypeRole).toUInt();

        if (leftPresence == rightPresence) {
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
    case AccountsModel::NicknameRole:
    {
        QString leftNickname = sourceModel()->data(left, AccountsModel::NicknameRole).toString();
        QString rightNickName = sourceModel()->data(right, AccountsModel::NicknameRole).toString();
        return QString::localeAwareCompare(leftNickname, rightNickName) < 0;
    }
    case AccountsModel::AliasRole:
    {
        QString leftAlias = sourceModel()->data(left, AccountsModel::AliasRole).toString();
        QString rightAlias = sourceModel()->data(right, AccountsModel::AliasRole).toString();
        return QString::localeAwareCompare(rightAlias, rightAlias) < 0;
    }
    case Qt::DisplayRole:
    default:
        return QString::localeAwareCompare(leftDisplayedName, rightDisplayedName) < 0;
    }
}


QModelIndexList AccountsFilterModel::match(const QModelIndex &start, int role,
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


#include "accounts-filter-model.moc"
