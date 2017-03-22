/*
 * Presence Model - A model of settable presences.
 *
 * Copyright (C) 2016 James D. Smith <smithjd15@gmail.com>
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
#include <QFont>
#include <QFontDatabase>
#include <QIcon>
#include <QtDBus/QtDBus>

#include <KSharedConfig>
#include <KLocalizedString>
#include <KConfig>
#include <KConfigGroup>

#include <functional>

#include "types.h"
#include "debug.h"

namespace KTp
{

struct PresenceModelGreaterThan : public std::function<bool(KTp::Presence,KTp::Presence)>
{
    inline bool operator()(const KTp::Presence& presence, const KTp::Presence& other)
    {
        if (KTp::Presence::sortPriority(presence.type()) < KTp::Presence::sortPriority(other.type())) {
            return true;
        } else if (KTp::Presence::sortPriority(presence.type()) == KTp::Presence::sortPriority(other.type())) {
            return (presence.statusMessage() < other.statusMessage());
        } else {
            return false;
        }
    }
};

PresenceModel::PresenceModel(QObject *parent) :
    QAbstractListModel(parent)
{
    Tp::registerTypes();

    loadPresences();

    QDBusConnection::sessionBus().connect(QString(), QLatin1String("/Telepathy"),
                                              QLatin1String("org.kde.Telepathy"),
                                              QLatin1String("presenceModelChanged"),
                                              this,
                                              SLOT(propagationChange(QVariantList)));
}

PresenceModel::~PresenceModel()
{
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

void PresenceModel::propagationChange(const QVariantList modelChange)
{
    KTp::Presence presence = KTp::Presence(qdbus_cast<Tp::SimplePresence>(modelChange.value(0)));
    bool presenceAdded = qdbus_cast<bool>(modelChange.value(1));

    if (!presence.isValid()) {
        return;
    }

    if (presenceAdded != m_presences.contains(presence)) {
        modifyModel(presence);
    }
}

QVariant PresenceModel::data(int index) const
{
    if (index < 0 || index >= m_presences.size()) {
        qCDebug(KTP_MODELS) << "invalid index data requested" << index;
        return QVariant();
    }

    return QVariant::fromValue<KTp::Presence>(m_presences[index]);
}

QVariant PresenceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        qCDebug(KTP_MODELS) << "invalid index data requested" << index;
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
            QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
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
    KSharedConfigPtr config = KSharedConfig::openConfig(QStringLiteral("ktelepathyrc"));
    config->reparseConfiguration();
    m_presenceGroup = config->group("Custom Presence List");
    m_presences.clear();
    loadDefaultPresences();
    loadCustomPresences();
}

void PresenceModel::loadDefaultPresences()
{
    modifyModel(KTp::Presence::available());
    modifyModel(KTp::Presence::busy());
    modifyModel(KTp::Presence::away());
    modifyModel(KTp::Presence::xa());
    modifyModel(KTp::Presence::hidden());
    modifyModel(KTp::Presence::offline());
}

void PresenceModel::loadCustomPresences()
{
    Q_FOREACH (const QString &key, m_presenceGroup.keyList()) {
        QVariantList entry = m_presenceGroup.readEntry(key, QVariantList());

        QString statusMessage = entry.last().toString();

        switch (entry.first().toInt()) {
        case Tp::ConnectionPresenceTypeAvailable:
            modifyModel(KTp::Presence::available(statusMessage));
            break;
        case Tp::ConnectionPresenceTypeAway:
            modifyModel(KTp::Presence::away(statusMessage));
            break;
        case Tp::ConnectionPresenceTypeBusy:
            modifyModel(KTp::Presence::busy(statusMessage));
            break;
        case Tp::ConnectionPresenceTypeExtendedAway:
            modifyModel(KTp::Presence::xa(statusMessage));
        }
    }
}

void PresenceModel::modifyModel(const KTp::Presence &presence)
{
    if (m_presences.contains(presence)) {
        int row = m_presences.indexOf(presence);
        beginRemoveRows(QModelIndex(), row, row);
        endRemoveRows();

        m_presences.removeOne(presence);
    } else {
        m_presences.append(presence);

        // Identical presence types with status messages are compared with the
        // status messages in ascending order.
        std::sort(m_presences.begin(), m_presences.end(), PresenceModelGreaterThan());

        int index = m_presences.indexOf(presence);

        beginInsertRows(QModelIndex(), index, index);
        endInsertRows();
    }

    Q_EMIT countChanged();
}

QModelIndex PresenceModel::addPresence(const KTp::Presence &presence)
{
    if (!m_presences.contains(presence)) {
        modifyModel(presence);
        propagateChange(presence);
    }

    return createIndex(m_presences.indexOf(presence), 0);
}

void PresenceModel::removePresence(const KTp::Presence &presence)
{
    if (m_presences.contains(presence)) {
        modifyModel(presence);
        propagateChange(presence);
    }
}

void PresenceModel::propagateChange(const KTp::Presence &presence)
{
    QVariantList messageArgList;
    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/Telepathy"),
                                                          QLatin1String("org.kde.Telepathy"),
                                                          QLatin1String("presenceModelChanged"));

    messageArgList << QVariant::fromValue<Tp::SimplePresence>(presence.barePresence());
    messageArgList << QVariant::fromValue<bool>(m_presences.contains(presence));
    message << messageArgList;

    if (!QDBusConnection::sessionBus().send(message)) {
        const QString &error = QDBusConnection::sessionBus().lastError().message();
        qCWarning(KTP_MODELS) << "presence model change propagation failed: " << error;
    }
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
