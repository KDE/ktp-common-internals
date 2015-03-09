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
#include <TelepathyQt/Constants>

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

QList<QAction*> KPeopleActionsPlugin::actionsForPerson(const KPeople::PersonData &person,
                                                       QObject *parent) const
{
    QList<QAction*> actions;

    // Get the most online account path and contact id
    QString personAccountPath = person.contactCustomProperty(QLatin1String("telepathy-accountPath")).toString();

    // The property for "telepathy-accountPath" is in form "/org/freedesktop/Telepathy/Account/gabble/jabber/myjabberaccount"
    // which is needed for retrieving the account from Tp::AccountManager. However we can only retrieve the other
    // contact uri list, not any objects which would give us the "telepathy-accountPath" for the subcontact.
    // So the code below constructs the account path from the contact uri which is known to be for ktp contacts
    // in form of "ktp:// + short account path + ? + contact id". But in order to not have the same actions
    // twice in the menu (one for most online and one from the contactUris()), this turns these properties into regular
    // uri format and puts it into the first position in the list, making sure that they will appear at the beggining.
    // It's also easier to simply remove it here and not having to check each uri separately in the foreach below.
    // And it cannot take the person.personUri() because for metacontacts that is in form of "kpeople://num_id".
    personAccountPath = personAccountPath.right(personAccountPath.length() - TP_QT_ACCOUNT_OBJECT_PATH_BASE.size() - 1);
    QString personContactId = person.contactCustomProperty(QLatin1String("telepathy-contactId")).toString();
    QString mostOnlineUri = QStringLiteral("ktp://") + personAccountPath + QLatin1Char('?') + personContactId;

    QStringList uris{mostOnlineUri};
    QStringList contactUris = person.contactUris();

    // Only append the child contacts if there is more than 1, otherwise
    // it means this contact has only itself as a subcontact.
    if (contactUris.size() > 1) {
        uris.append(contactUris);
        // Make sure we don't have duplicate uris in the list
        uris.removeDuplicates();
    }

    Q_FOREACH (const QString &uri, uris) {
        if (!uri.startsWith(QStringLiteral("ktp://"))) {
            continue;
        }

        int delimiterIndex = uri.indexOf(QLatin1Char('?'));
        QString contactId = uri.right(uri.length() - delimiterIndex - 1);
        QString accountPath = uri.mid(6, delimiterIndex - 6);
        // Prepend the "/org/freedesktop/Telepathy" part so that Tp::AccountManager
        // returns valid account
        accountPath.prepend(TP_QT_ACCOUNT_OBJECT_PATH_BASE + QLatin1Char('/'));


        const Tp::AccountPtr account = KTp::contactManager()->accountForAccountPath(accountPath);
        if (!account) {
            continue;
        }

        const KTp::ContactPtr contact = KTp::contactManager()->contactForContactId(accountPath, contactId);
        if (!contact || !contact->manager()) {
            continue;
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
    }

//     QAction *action = new IMAction(i18n("Open Log Viewer..."),
//                                    QIcon::fromTheme(QStringLiteral("documentation")),
//                                    person.personUri(),
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
