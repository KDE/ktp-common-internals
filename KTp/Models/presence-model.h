/*
 * Presence Model - A model of settable presences.
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

#ifndef PRESENCEMODEL_H
#define PRESENCEMODEL_H

#include <QAbstractListModel>

#include <KConfigGroup>

#include <KTp/presence.h>

namespace KTp
{

class KTP_EXPORT PresenceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
public:
    explicit PresenceModel(QObject *parent = 0);
    ~PresenceModel();

    enum Roles {
        //Also supplies Qt::DisplayRole and Qt::DecorationRole
        PresenceRole = Qt::UserRole,
        IconNameRole
    };

    /** Adds a custom presence to the model, and write value to config file.
      @return the newly added item
    */
    QModelIndex addPresence(const KTp::Presence &presence);

    void removePresence(const KTp::Presence &presence);

    /** Load all presences from disk */
    void loadPresences();

    /** Write all presences to disk */
    void syncCustomPresencesToDisk();

    /** Updates context menu of presence applet */
    int updatePresenceApplet();

    Q_SCRIPTABLE QVariant get(int row, const QByteArray& role) const;

    //protected:
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant data(int index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QHash<int, QByteArray> roleNames() const;

Q_SIGNALS:
    void countChanged();

private:

    /** Loads standard presences (online, away etc) into */
    void loadDefaultPresences();

    /** Loads any user custom presences into the model */
    void loadCustomPresences();

    QList<KTp::Presence> m_presences;

    //this is wrong, KConfigGroup is a sharedptr..
    KConfigGroup m_presenceGroup;
};

}

#endif // PRESENCEMODEL_H
