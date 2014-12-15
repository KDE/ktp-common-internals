/*
 * Model of all accounts with inbuilt grouping and filtering
 *
 * Copyright (C) 2013 David Edmundson <kde@davidedmundson.co.uk>
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

#ifndef KTP_CONTACTS_MODEL_H
#define KTP_CONTACTS_MODEL_H

#include <KTp/Models/contacts-filter-model.h>
#include <KTp/types.h>
#include <KTp/Models/ktpmodels_export.h>

namespace KTp
{

/** This class combines the list model and all the relevant filtering into a simple to use class
    In most cases you should use this as the entry point to the models in your application
 */

class KTPMODELS_EXPORT ContactsModel : public KTp::ContactsFilterModel
{
    Q_OBJECT
    Q_PROPERTY(GroupMode groupMode READ groupMode WRITE setGroupMode NOTIFY groupModeChanged)
    Q_PROPERTY(bool trackUnreadMessages READ trackUnreadMessages WRITE setTrackUnreadMessages NOTIFY trackUnreadMessagesChanged)

    Q_PROPERTY(Tp::AccountManagerPtr accountManager READ accountManager WRITE setAccountManager)

    Q_ENUMS(GroupMode)

public:
    enum GroupMode {
        /** Contacts are not grouped and are a simple flat list*/
        NoGrouping,
        /** Contacts are grouped by their account using AccountsTreeProxyModel*/
        AccountGrouping,
        /** Contacts are grouped by their group name using GroupsTreeProxyModel*/
        GroupGrouping
    };

    ContactsModel(QObject *parent=0);

    virtual ~ContactsModel();

    /** Sets the accounts manager to be used for the KTp::ContactListModel*/
    void setAccountManager(const Tp::AccountManagerPtr &accountManager);

    /** Returns account manager currently used by the model */
    Tp::AccountManagerPtr accountManager() const;

    /** Specify how the contacts should be grouped together*/
    void setGroupMode(GroupMode mode);

    /** The currently specified grouping model*/
    GroupMode groupMode() const;

    /** Specify whether to track unread messages or not. Note this adds additional overhead, so only use it if you're going to show the information
      * Default is False
    */
    void setTrackUnreadMessages(bool trackUnread);
    bool trackUnreadMessages() const;
    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

Q_SIGNALS:
    void modelInitialized(bool success);
    void groupModeChanged();
    void trackUnreadMessagesChanged();

private:
    class Private;
    Private *d;

    void updateGroupProxyModels();
};

}

#endif // KTP_CONTACTS_MODEL_H
