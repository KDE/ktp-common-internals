/*
    Copyright (C) 2013  David Edmundson <davidedmundson@kde.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "kpeople-actions-plugin.h"

#include <QAction>

#include <QIcon>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QFileDialog>

#include "KTp/contact.h"
#include "KTp/actions.h"
#include "KTp/core.h"
#include "KTp/global-contact-manager.h"

#include <TelepathyQt/Account>
#include <TelepathyQt/ContactManager>

#include <KPeople/PersonData>

enum IMActionType {
    TextChannel,
    AudioChannel,
    VideoChannel,
    FileTransfer,
    LogViewer,
    CollabEditing
};

class IMAction : public QAction {
    Q_OBJECT
public:
    IMAction(const QString &text, const QIcon &icon, const KTp::ContactPtr &contact,
             const Tp::AccountPtr &account, IMActionType type, QObject *parent);
    IMAction(const QString &text, const QIcon &icon, const QUrl &uri,
             IMActionType type, QObject *parent);
    KTp::ContactPtr contact() const;
    Tp::AccountPtr account() const;
    IMActionType type() const;
    QUrl uri() const;
private:
    KTp::ContactPtr m_contact;
    Tp::AccountPtr m_account;
    QUrl m_uri;
    IMActionType m_type;
};

IMAction::IMAction(const QString &text, const QIcon &icon, const KTp::ContactPtr &contact,
                   const Tp::AccountPtr &account, IMActionType type, QObject *parent):
    QAction(icon, text, parent),
    m_contact(contact),
    m_account(account),
    m_type(type)
{
}

IMAction::IMAction(const QString &text, const QIcon &icon, const QUrl &uri,
                   IMActionType type, QObject *parent):
    QAction(icon, text, parent),
    m_uri(uri),
    m_type(type)
{
}

KTp::ContactPtr IMAction::contact() const
{
    return m_contact;
}

Tp::AccountPtr IMAction::account() const
{
    return m_account;
}

IMActionType IMAction::type() const
{
    return m_type;
}

QUrl IMAction::uri() const
{
    return m_uri;
}

KPeopleActionsPlugin::KPeopleActionsPlugin(QObject *parent, const QVariantList &args)
    : AbstractPersonAction(parent)
{
    Q_UNUSED(args);
}

QList<QAction*> KPeopleActionsPlugin::actionsForPerson(const KContacts::Addressee &person,
                                                       const KContacts::Addressee::List &contacts,
                                                       QObject *parent) const
{
    QList<QAction*> actions;

    // === TODO ===
    // This creates actions just for the "most online contact", what we want is to query all
    // the subcontacts for all capabilities and fill them in on the Person, so if eg. one of
    // the subcontacts can do audio calls and the other can do video calls, the Person
    // should have both actions present.
    Q_UNUSED(contacts);

    const QString &accountPath = person.custom(QLatin1String("telepathy"), QLatin1String("accountPath"));
    const QString &contactId = person.custom(QLatin1String("telepathy"), QLatin1String("contactId"));

    const Tp::AccountPtr account = KTp::contactManager()->accountForAccountPath(accountPath);
    if (!account) {
        return actions;
    }

    const KTp::ContactPtr contact = KTp::contactManager()->contactForContactId(accountPath, contactId);
    if (!contact || !contact->manager()) {
        return actions;
    }

    if (contact->textChatCapability()) {
        QAction *action = new IMAction(i18n("Start Chat Using %1...", account->displayName()),
                            QIcon::fromTheme(QStringLiteral("text-x-generic")),
                            contact,
                            account,
                            TextChannel,
                            parent);
        connect (action, SIGNAL(triggered(bool)), SLOT(onActionTriggered()));
        actions << action;
    }
    if (contact->audioCallCapability()) {
        QAction *action = new IMAction(i18n("Start Audio Call Using %1...", account->displayName()),
                            QIcon::fromTheme(QStringLiteral("audio-headset")),
                            contact,
                            account,
                            AudioChannel,
                            parent);
        connect (action, SIGNAL(triggered(bool)), SLOT(onActionTriggered()));
        actions << action;
    }
    if (contact->videoCallCapability()) {
        QAction *action = new IMAction(i18n("Start Video Call Using %1...", account->displayName()),
                            QIcon::fromTheme(QStringLiteral("camera-web")),
                            contact,
                            account,
                            VideoChannel,
                            parent);
        connect (action, SIGNAL(triggered(bool)), SLOT(onActionTriggered()));
        actions << action;
    }

    if (contact->fileTransferCapability()) {
        QAction *action = new IMAction(i18n("Send Files Using %1...", account->displayName()),
                                    QIcon::fromTheme(QStringLiteral("mail-attachment")),
                                    contact,
                                    account,
                                    FileTransfer,
                                    parent);
        connect (action, SIGNAL(triggered(bool)), SLOT(onActionTriggered()));
        actions << action;
    }
    if (contact->collaborativeEditingCapability()) {
        QAction *action = new IMAction(i18n("Collaboratively edit a document Using %1...", account->displayName()),
                                    QIcon::fromTheme(QStringLiteral("document-edit")),
                                    contact,
                                    account,
                                    CollabEditing,
                                    parent);
        connect (action, SIGNAL(triggered(bool)), SLOT(onActionTriggered()));
        actions << action;
    }

    //FIXME-KPEOPLE
//     QAction *action = new IMAction(i18n("Open Log Viewer..."),
//                                    QIcon::fromTheme(QStringLiteral("documentation")),
//                                    personData->uri(),
//                                    LogViewer,
//                                    parent);
//     connect(action, SIGNAL(triggered(bool)), SLOT(onActionTriggered()));
//     actions << action;
    return actions;
}

void KPeopleActionsPlugin::onActionTriggered()
{
    IMAction *action = qobject_cast<IMAction*>(sender());
    KTp::ContactPtr contact = action->contact();
    Tp::AccountPtr account = action->account();
    IMActionType type = action->type();

    switch (type) {
        case TextChannel:
            KTp::Actions::startChat(account, contact);
            break;
        case AudioChannel:
            KTp::Actions::startAudioCall(account, contact);
            break;
        case VideoChannel:
            KTp::Actions::startAudioVideoCall(account, contact);
            break;
        case FileTransfer: {
            const QStringList fileNames = QFileDialog::getOpenFileNames(Q_NULLPTR, i18n("Choose files to send to %1", contact->alias()),
                                                                        QStringLiteral("kfiledialog:///FileTransferLastDirectory"));
            Q_FOREACH(const QString& file, fileNames) {
                KTp::Actions::startFileTransfer(account, contact, file);
            }
            break;
        }
        case LogViewer:
            KTp::Actions::openLogViewer(action->uri());
            break;
        case CollabEditing: {
            const QUrl file = QFileDialog::getOpenFileName(Q_NULLPTR, i18n("Choose a file to edit with %1", contact->alias()),
                                                           QStringLiteral("kfiledialog:///CollabEditingLastDirectory"));
            KTp::Actions::startCollaborativeEditing(account, contact, QList<QUrl>() << file, true);
            break;
        }
    }
}

K_PLUGIN_FACTORY( KPeopleActionsPluginFactory, registerPlugin<KPeopleActionsPlugin>(); )
K_EXPORT_PLUGIN( KPeopleActionsPluginFactory("ktp_kpeople_plugin", "ktp-common-internals") )

#include "kpeople-actions-plugin.moc"
