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

#include "presence-model.h"

#include <QString>
#include <QtGui/QFont>
#include <QtDBus/QtDBus>

#include <KDE/KIcon>
#include <KDE/KLocalizedString>
#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KGlobalSettings>
#include <KDE/KDebug>

namespace KTp
{

PresenceModel::PresenceModel(QObject *parent) :
    QAbstractListModel(parent)
{
    loadPresences();
}

PresenceModel::~PresenceModel()
{
    syncCustomPresencesToDisk();
}

void PresenceModel::syncCustomPresencesToDisk()
{
    m_presenceGroup.deleteGroup();

    Q_FOREACH (const KTp::Presence &presence, m_presences) {
        if (!presence.statusMessage().isEmpty()) {
            QVariantList presenceVariant;
            presenceVariant.append(presence.type());
            presenceVariant.append(presence.statusMessage());
            QString id = QString::number(presence.type()).append(presence.statusMessage());
            m_presenceGroup.writeEntry(id, presenceVariant);
        }
    }
    m_presenceGroup.sync();
}

QVariant PresenceModel::data(int index) const
{
    if (index < 0 || index >= m_presences.size()) {
        kDebug() << "invalid index data requested" << index;
        return QVariant();
    }

    return QVariant::fromValue<KTp::Presence>(m_presences[index]);
}

QVariant PresenceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        kDebug() << "invalid index data requested" << index;
        return QVariant();
    }

    KTp::Presence presence = m_presences[index.row()];
    switch (role) {
    case Qt::DisplayRole:
        if (presence.statusMessage().isEmpty()) {
            return presence.displayString();
        } else {
            return presence.statusMessage();
        }

    case Qt::DecorationRole:
        return presence.icon();

    case Qt::FontRole:
        if (presence.statusMessage().isEmpty()) {
            QFont font = KGlobalSettings::generalFont();
            font.setBold(true);
            return font;
        } else {
            return QVariant();
        }

    case PresenceRole:
        return QVariant::fromValue<KTp::Presence>(presence);

    case IconNameRole:
        return presence.iconName();
    }

    return QVariant();
}

int PresenceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_presences.size();
}

void PresenceModel::loadPresences()
{
    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
    config->reparseConfiguration();
    m_presenceGroup = config->group("Custom Presence List");
    m_presences.clear();
    loadDefaultPresences();
    loadCustomPresences();
}

void PresenceModel::loadDefaultPresences()
{
    addPresence(Tp::Presence::available());
    addPresence(Tp::Presence::busy());
    addPresence(Tp::Presence::away());
    addPresence(Tp::Presence::xa());
    addPresence(Tp::Presence::hidden());
    addPresence(Tp::Presence::offline());
}

void PresenceModel::loadCustomPresences()
{
    Q_FOREACH (const QString &key, m_presenceGroup.keyList()) {
        QVariantList entry = m_presenceGroup.readEntry(key, QVariantList());

        QString statusMessage = entry.last().toString();

        switch (entry.first().toInt()) {
        case Tp::ConnectionPresenceTypeAvailable:
            addPresence(Tp::Presence::available(statusMessage));
            break;
        case Tp::ConnectionPresenceTypeAway:
            addPresence(Tp::Presence::away(statusMessage));
            break;
        case Tp::ConnectionPresenceTypeBusy:
            addPresence(Tp::Presence::busy(statusMessage));
            break;
        case Tp::ConnectionPresenceTypeExtendedAway:
            addPresence(Tp::Presence::xa(statusMessage));
        }
    }
}

QModelIndex PresenceModel::addPresence(const KTp::Presence &presence)
{
    if (m_presences.contains(presence)) {
        return createIndex(m_presences.indexOf(presence), 0);
    }

    QList<KTp::Presence>::iterator i = qLowerBound(m_presences.begin(), m_presences.end(), KTp::Presence(presence));
    m_presences.insert(i, presence);

    int index = m_presences.indexOf(presence);
    //this is technically a backwards and wrong, but I can't get a row from a const iterator,
    //and using qLowerBound does seem a good approach
    beginInsertRows(QModelIndex(), index, index);
    endInsertRows();
    Q_EMIT countChanged();
    return createIndex(index, 0);
}

void PresenceModel::removePresence(const KTp::Presence &presence)
{
    int row = m_presences.indexOf(presence);
    beginRemoveRows(QModelIndex(), row, row);
    m_presences.removeOne(presence);
    endRemoveRows();
    Q_EMIT countChanged();
}

int PresenceModel::updatePresenceApplet()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        return 1;
    }

    QDBusInterface callApplet(QLatin1String("org.kde.Telepathy.PresenceAppletActive"),
                              QLatin1String("/"), QLatin1String(""), QDBusConnection::sessionBus());
    if (callApplet.isValid()) {
        callApplet.asyncCall(QLatin1String("handleCustomPresenceChange"));
        return 0;
    }
    return 1;
}

QHash<int, QByteArray> PresenceModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles.insert(PresenceRole, "presence");
    roles.insert(IconNameRole, "iconName");
    return roles;
}

QVariant PresenceModel::get(int row, const QByteArray& role) const
{
    //TODO: cache roles?
    QHash<int, QByteArray> roles = roleNames();
    return index(row, 0).data(roles.key(role));
}

}
