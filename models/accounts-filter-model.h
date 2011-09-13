/*
 * Provide some filters on the account model
 *
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
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

class AccountsModelItem;
class ContactModelItem;

/**
  * \brief Class used to sort and filter the contacts.
  *
  * Filters:
  *     Hide offline contacts
  *     Hide contacts not matching a string in the search bar
  * Sort contacts:
  *     By name
  *     By presence
  */
class AccountsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    AccountsFilterModel(QObject *parent = 0);

    bool showOfflineUsers() const;

    /**
     * \brief Flag to sort the contactlist by presence.
     *
     * If set to false, the contact list is only sorted by name.
     */
    bool isSortedByPresence() const;

public slots:
    void setShowOfflineUsers(bool showOfflineUsers);
    void setFilterString(const QString &str);
    /// This overloaded method takes into account showing/hiding offline users
    void clearFilterStringAndHideOfflineUsers(const bool& showOfflineUsers);
    void clearFilterString();
    /**
     * \brief Lets the proxy know whether the model should get sorted by presence or not.
     *
     * \param enabled if true, the model will get sorted by presence, otherwise just by name.
     */
    void setSortByPresence(bool enabled);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan ( const QModelIndex &left, const QModelIndex &right ) const;

private:

    bool filterAcceptsAccount(const QModelIndex &index) const;
    bool filterAcceptsContact(const QModelIndex &index) const;
    bool filterAcceptsGroup(const QModelIndex &index) const;

    /// Shows offline users
    bool m_showOfflineUsers;

    /// Used when searching for contact
    bool m_filterByName;

    /// Holds the string which is searched in the model
    QString m_filterString;

    /// Sets the sorting order of presences
    QHash<uint, int> m_presenceSorting;
};

#endif // ACCOUNTFILTERMODEL_H
