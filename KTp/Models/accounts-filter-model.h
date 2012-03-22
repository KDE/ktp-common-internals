/*
 * Provide some filters on the account model
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
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

#ifndef ACCOUNTSFILTERMODEL_H
#define ACCOUNTSFILTERMODEL_H

#include <QSortFilterProxyModel>

#include <KTp/ktp-export.h>

class AccountsModelItem;
class ContactModelItem;

/**
  * \brief Class used to sort and filter the contacts.
  */
class KTP_EXPORT AccountsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_DISABLE_COPY(AccountsFilterModel)

    Q_ENUMS(SortMode
            PresenceFilterFlag
            CapabilityFilterFlag
            SubscriptionStateFilterFlag)

    Q_FLAGS(PresenceTypeFilterFlags
            CapabilityFilterFlags
            SubscriptionStateFilterFlags)

    Q_PROPERTY(SortMode sortMode
               READ sortMode
               WRITE setSortMode
               NOTIFY sortModeChanged)

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

public:
    enum SortMode {
        DoNotSort = 0,
        SortByPresence,
        SortByNickname,
        SortByAlias
    };

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
        FilterByMediaCallCapability            = 0x0002,
        FilterByAudioCallCapability            = 0x0004,
        FilterByVideoCallCapability            = 0x0008,
        FilterByFileTransferCapability         = 0x0010,
        FilterByDesktopSharingCapability       = 0x0020,
        FilterBySSHContactCapability           = 0x0040,

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

    AccountsFilterModel(QObject *parent = 0);
    virtual ~AccountsFilterModel();

    SortMode sortMode() const;
    void resetSortMode();
    Q_SLOT void setSortMode(SortMode sortMode);
    Q_SIGNAL void sortModeChanged(SortMode sortMode);

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

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan ( const QModelIndex &left, const QModelIndex &right ) const;
QModelIndexList match(const QModelIndex &start, int role,
                                          const QVariant &value, int hits,
                                          Qt::MatchFlags flags) const;

private:
    class Private;
    Private * const d;
};

#endif // ACCOUNTFILTERMODEL_H
