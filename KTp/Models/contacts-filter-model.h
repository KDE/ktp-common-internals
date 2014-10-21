/*
 * Provide some filters on the account model
 *
 * Copyright (C) 2011,2012 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2012 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
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

#ifndef CONTACTSFILTERMODEL_H
#define CONTACTSFILTERMODEL_H

#include <QSortFilterProxyModel>

#include <KTp/types.h>
#include <KTp/Models/ktpmodels_export.h>

namespace KTp

{

/**
  * \brief Class used to sort and filter the contacts.
  */
class KTPMODELS_EXPORT ContactsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactsFilterModel)

    Q_ENUMS(PresenceFilterFlag
            CapabilityFilterFlag
            SubscriptionStateFilterFlag)

    Q_FLAGS(PresenceTypeFilterFlags
            CapabilityFilterFlags
            SubscriptionStateFilterFlags)

    Q_PROPERTY(PresenceTypeFilterFlags presenceTypeFilterFlags
               READ presenceTypeFilterFlags
               RESET clearPresenceTypeFilterFlags
               WRITE setPresenceTypeFilterFlags
               NOTIFY presenceTypeFilterFlagsChanged)

    Q_PROPERTY(CapabilityFilterFlags capabilityFilterFlags
               READ capabilityFilterFlags
               RESET clearCapabilityFilterFlags
               WRITE setCapabilityFilterFlags
               NOTIFY capabilityFilterFlagsChanged)

    // FIXME This resumes Subscription state, Publish state and blocking state, perhaps we should find a better name
    Q_PROPERTY(SubscriptionStateFilterFlags subscriptionStateFilterFlags
               READ subscriptionStateFilterFlags
               RESET clearSubscriptionStateFilterFlags
               WRITE setSubscriptionStateFilterFlags
               NOTIFY subscriptionStateFilterFlagsChanged)

    // Filters on all fields
    Q_PROPERTY(QString globalFilterString
               READ globalFilterString
               RESET clearGlobalFilterString
               WRITE setGlobalFilterString
               NOTIFY globalFilterStringChanged)
    Q_PROPERTY(Qt::MatchFlags globalFilterMatchFlags
               READ globalFilterMatchFlags
               RESET resetGlobalFilterMatchFlags
               WRITE setGlobalFilterMatchFlags
               NOTIFY globalFilterMatchFlagsChanged)

    Q_PROPERTY(QString displayNameFilterString
               READ displayNameFilterString
               RESET clearDisplayNameFilterString
               WRITE setDisplayNameFilterString
               NOTIFY displayNameFilterStringChanged)
    Q_PROPERTY(QString nicknameFilterString
               READ nicknameFilterString
               RESET clearNicknameFilterString
               WRITE setNicknameFilterString
               NOTIFY nicknameFilterStringChanged)
    Q_PROPERTY(QString aliasFilterString
               READ aliasFilterString
               RESET clearAliasFilterString
               WRITE setAliasFilterString
               NOTIFY aliasFilterStringChanged)
    Q_PROPERTY(QString groupsFilterString
               READ groupsFilterString
               RESET clearGroupsFilterString
               WRITE setGroupsFilterString
               NOTIFY groupsFilterStringChanged)
    Q_PROPERTY(QString idFilterString
               READ idFilterString
               RESET clearIdFilterString
               WRITE setIdFilterString
               NOTIFY idFilterStringChanged)
    Q_PROPERTY(QStringList tubesFilterStrings
               READ tubesFilterStrings
               RESET clearTubesFilterStrings
               WRITE setTubesFilterStrings
               NOTIFY tubesFilterStringsChanged)
    Q_PROPERTY(Qt::MatchFlags displayNameFilterMatchFlags
               READ displayNameFilterMatchFlags
               RESET resetDisplayNameFilterMatchFlags
               WRITE setDisplayNameFilterMatchFlags
               NOTIFY displayNameFilterMatchFlagsChanged)
    Q_PROPERTY(Qt::MatchFlags nicknameFilterMatchFlags
               READ nicknameFilterMatchFlags
               RESET resetNicknameFilterMatchFlags
               WRITE setNicknameFilterMatchFlags
               NOTIFY nicknameFilterMatchFlagsChanged)
    Q_PROPERTY(Qt::MatchFlags aliasFilterMatchFlags
               READ aliasFilterMatchFlags
               RESET resetAliasFilterMatchFlags
               WRITE setAliasFilterMatchFlags
               NOTIFY aliasFilterMatchFlagsChanged)
    Q_PROPERTY(Qt::MatchFlags groupsFilterMatchFlags
               READ groupsFilterMatchFlags
               RESET resetGroupsFilterMatchFlags
               WRITE setGroupsFilterMatchFlags
               NOTIFY groupsFilterMatchFlagsChanged)
    Q_PROPERTY(Qt::MatchFlags idFilterMatchFlags
               READ idFilterMatchFlags
               RESET resetIdFilterMatchFlags
               WRITE setIdFilterMatchFlags
               NOTIFY idFilterMatchFlagsChanged)

    Q_PROPERTY(Tp::AccountPtr accountFilter
               READ accountFilter
               RESET clearAccountFilter
               WRITE setAccountFilter
               NOTIFY accountFilterChanged)

    Q_PROPERTY(QString sortRoleString
               READ sortRoleString
               WRITE setSortRoleString)

public:

    enum PresenceTypeFilterFlag {
        DoNotFilterByPresence                  = 0x0000,
        HidePresenceTypeUnset                  = 0x0001,
        HidePresenceTypeOffline                = 0x0002,
        HidePresenceTypeAvailable              = 0x0004,
        HidePresenceTypeAway                   = 0x0008,
        HidePresenceTypeExtendedAway           = 0x0010,
        HidePresenceTypeHidden                 = 0x0020,
        HidePresenceTypeBusy                   = 0x0040,
        HidePresenceTypeUnknown                = 0x0080,
        HidePresenceTypeError                  = 0x0100,
        HideAllOffline = HidePresenceTypeUnset |
                         HidePresenceTypeOffline |
                         HidePresenceTypeUnknown |
                         HidePresenceTypeError,
        HideAllOnline = HidePresenceTypeAvailable |
                        HidePresenceTypeAway |
                        HidePresenceTypeExtendedAway |
                        HidePresenceTypeHidden |
                        HidePresenceTypeBusy,
        HideAllUnavailable = HideAllOffline |
                             HidePresenceTypeAway |
                             HidePresenceTypeExtendedAway |
                             HidePresenceTypeBusy,
        ShowOnlyConnected = HideAllOffline,
        ShowOnlyDisconnected = HideAllOnline,
        ShowAll = DoNotFilterByPresence
    };
    Q_DECLARE_FLAGS(PresenceTypeFilterFlags, PresenceTypeFilterFlag)

    enum CapabilityFilterFlag {
        DoNotFilterByCapability                = 0x0000,
        FilterByTextChatCapability             = 0x0001,
        FilterByAudioCallCapability            = 0x0002,
        FilterByVideoCallCapability            = 0x0004,
        FilterByFileTransferCapability         = 0x0008,
        FilterByTubes                          = 0x0010,

        CustomFilterCapability                 = 0x10000 // a placemark for custom capabilities in inherited classes
    };
    Q_DECLARE_FLAGS(CapabilityFilterFlags, CapabilityFilterFlag)

    enum SubscriptionStateFilterFlag {
        DoNotFilterBySubscription              = 0x0000,
        HideSubscriptionStateNo                = 0x0001,
        HideSubscriptionStateAsk               = 0x0002,
        HideSubscriptionStateYes               = 0x0004,
        HidePublishStateNo                     = 0x0010,
        HidePublishStateAsk                    = 0x0020,
        HidePublishStateYes                    = 0x0040,
        HideBlocked                            = 0x0100,
        HideNonBlocked                         = 0x0200,
        ShowOnlyBlocked = HideNonBlocked
    };
    Q_DECLARE_FLAGS(SubscriptionStateFilterFlags, SubscriptionStateFilterFlag)

    ContactsFilterModel(QObject *parent = 0);
    virtual ~ContactsFilterModel();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual void setSourceModel(QAbstractItemModel *sourceModel);
    void invalidateFilter();

    PresenceTypeFilterFlags presenceTypeFilterFlags() const;
    Q_SLOT void clearPresenceTypeFilterFlags();
    Q_SLOT void setPresenceTypeFilterFlags(PresenceTypeFilterFlags presenceTypeFilterFlags);
    Q_SIGNAL void presenceTypeFilterFlagsChanged(PresenceTypeFilterFlags presenceTypeFilterFlags);

    CapabilityFilterFlags capabilityFilterFlags() const;
    Q_SLOT void clearCapabilityFilterFlags();
    Q_SLOT void setCapabilityFilterFlags(CapabilityFilterFlags capabilityFilterFlags);
    Q_SIGNAL void capabilityFilterFlagsChanged(CapabilityFilterFlags capabilityFilterFlags);

    SubscriptionStateFilterFlags subscriptionStateFilterFlags() const;
    Q_SLOT void clearSubscriptionStateFilterFlags();
    Q_SLOT void setSubscriptionStateFilterFlags(SubscriptionStateFilterFlags subscriptionStateFilterFlags);
    Q_SIGNAL void subscriptionStateFilterFlagsChanged(SubscriptionStateFilterFlags subscriptionStateFilterFlags);

    QString globalFilterString() const;
    Q_SLOT void clearGlobalFilterString();
    Q_SLOT void setGlobalFilterString(const QString &globalFilterString);
    Q_SIGNAL void globalFilterStringChanged(const QString &globalFilterString);
    Qt::MatchFlags globalFilterMatchFlags() const;
    Q_SLOT void resetGlobalFilterMatchFlags();
    Q_SLOT void setGlobalFilterMatchFlags(Qt::MatchFlags globalStringMatchFlags);
    Q_SIGNAL void globalFilterMatchFlagsChanged(Qt::MatchFlags globalStringMatchFlags);

    QString displayNameFilterString() const;
    Q_SLOT void clearDisplayNameFilterString();
    Q_SLOT void setDisplayNameFilterString(const QString &displayNameFilterString);
    Q_SIGNAL void displayNameFilterStringChanged(const QString &displayNameFilterString);
    Qt::MatchFlags displayNameFilterMatchFlags() const;
    Q_SLOT void resetDisplayNameFilterMatchFlags();
    Q_SLOT void setDisplayNameFilterMatchFlags(Qt::MatchFlags displayNameFilterMatchFlags);
    Q_SIGNAL void displayNameFilterMatchFlagsChanged(Qt::MatchFlags displayNameFilterMatchFlags);

    QString nicknameFilterString() const;
    Q_SLOT void clearNicknameFilterString();
    Q_SLOT void setNicknameFilterString(const QString &nicknameFilterString);
    Q_SIGNAL void nicknameFilterStringChanged(const QString &nicknameFilterString);
    Qt::MatchFlags nicknameFilterMatchFlags() const;
    Q_SLOT void resetNicknameFilterMatchFlags();
    Q_SLOT void setNicknameFilterMatchFlags(Qt::MatchFlags nicknameFilterMatchFlags);
    Q_SIGNAL void nicknameFilterMatchFlagsChanged(Qt::MatchFlags nicknameFilterMatchFlags);

    QString aliasFilterString() const;
    Q_SLOT void clearAliasFilterString();
    Q_SLOT void setAliasFilterString(const QString &aliasFilterString);
    Q_SIGNAL void aliasFilterStringChanged(const QString &aliasFilterString);
    Qt::MatchFlags aliasFilterMatchFlags() const;
    Q_SLOT void resetAliasFilterMatchFlags();
    Q_SLOT void setAliasFilterMatchFlags(Qt::MatchFlags aliasFilterMatchFlags);
    Q_SIGNAL void aliasFilterMatchFlagsChanged(Qt::MatchFlags aliasFilterMatchFlags);

    QString groupsFilterString() const;
    Q_SLOT void clearGroupsFilterString();
    Q_SLOT void setGroupsFilterString(const QString &groupsFilterString);
    Q_SIGNAL void groupsFilterStringChanged(const QString &groupsFilterString);
    Qt::MatchFlags groupsFilterMatchFlags() const;
    Q_SLOT void resetGroupsFilterMatchFlags();
    Q_SLOT void setGroupsFilterMatchFlags(Qt::MatchFlags groupsFilterMatchFlags);
    Q_SIGNAL void groupsFilterMatchFlagsChanged(Qt::MatchFlags groupsFilterMatchFlags);

    QString idFilterString() const;
    Q_SLOT void clearIdFilterString();
    Q_SLOT void setIdFilterString(const QString &idFilterString);
    Q_SIGNAL void idFilterStringChanged(const QString &idFilterString);
    Qt::MatchFlags idFilterMatchFlags() const;
    Q_SLOT void resetIdFilterMatchFlags();
    Q_SLOT void setIdFilterMatchFlags(Qt::MatchFlags idFilterMatchFlags);
    Q_SIGNAL void idFilterMatchFlagsChanged(Qt::MatchFlags idFilterMatchFlags);

    Tp::AccountPtr accountFilter() const;
    Q_SLOT void clearAccountFilter();
    Q_SLOT void setAccountFilter(const Tp::AccountPtr &accountFilter);
    Q_SIGNAL void accountFilterChanged(const Tp::AccountPtr &accountFilter);

    QStringList tubesFilterStrings() const;
    Q_SLOT void clearTubesFilterStrings();
    Q_SLOT void setTubesFilterStrings(const QStringList &tubesFilterStrings);
    Q_SIGNAL void tubesFilterStringsChanged(const QStringList &tubesFilterStrings);

    QString sortRoleString() const;
    Q_SLOT void setSortRoleString(const QString &role);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan (const QModelIndex &left, const QModelIndex &right) const;
    QModelIndexList match(const QModelIndex &start, int role,
                          const QVariant &value, int hits,
                          Qt::MatchFlags flags) const;

private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void sourceModelParentIndexChanged(const QModelIndex &sourceIndex))
    Q_PRIVATE_SLOT(d, void sourceModelIndexChanged(const QModelIndex &sourceIndex))
};
} //namespace

#endif // CONTACTSFILTERMODEL_H
