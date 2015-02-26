/*
    Copyright (C) 2012 Aleix Pol <aleixpol@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef PINNEDCONTACTSMODEL_H
#define PINNEDCONTACTSMODEL_H

#include <QModelIndex>
#include <QVector>

#include <KTp/types.h>
#include <KTp/persistent-contact.h>

class ConversationsModel;
class PinnedContactsModelPrivate;

class PinnedContactsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(ConversationsModel *conversations READ conversationsModel WRITE setConversationsModel)
    Q_PROPERTY(Tp::AccountManagerPtr accountManager READ accountManager WRITE setAccountManager)
    Q_PROPERTY(QStringList state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

  public:
    explicit PinnedContactsModel(QObject *parent = 0);
    virtual ~PinnedContactsModel();

    enum role {
        PresenceIconRole = Qt::UserRole + 1,
        AvailabilityRole,
        ContactRole,
        AccountRole,
        AlreadyChattingRole
    };

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    Q_SLOT void setPinning(const Tp::AccountPtr &account, const KTp::ContactPtr &contact, bool newState);

    QModelIndex indexForContact(const KTp::ContactPtr &contact) const;

    ConversationsModel* conversationsModel() const;
    void setConversationsModel(ConversationsModel *model);

    Tp::AccountManagerPtr accountManager() const;
    void setAccountManager(const Tp::AccountManagerPtr &accounts);

    QStringList state() const;
    void setState(const QStringList &s);

  private Q_SLOTS:
    void contactDataChanged();
    void contactChanged(const KTp::ContactPtr &contact);
    void conversationsStateChanged(const QModelIndex &parent, int start, int end);
    void onAccountManagerReady();

  Q_SIGNALS:
    void countChanged();
    void stateChanged();

  private:
    void appendContactPin(const KTp::PersistentContactPtr &pin);
    void removeContactPin(const KTp::PersistentContactPtr &pin);
    PinnedContactsModelPrivate * const d;
};

#endif // PINNEDCONTACTSMODEL_H
