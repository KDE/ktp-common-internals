/*
    Copyright (C) 2013 David Edmundson <davidedmundson@kde.org>
    Copyright (C) 2013 Aleix Pol <aleixpol@kde.org>

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

#ifndef TELEPATHYMANAGER_H
#define TELEPATHYMANAGER_H

#include <QObject>
#include <TelepathyQt/Types>

#include <KTp/contact.h>
#include <KTp/contact-factory.h>

#include <KTp/types.h>

namespace Tp {
class PendingChannelRequest;
}


class TelepathyManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Tp::AccountManagerPtr accountManager READ accountManager CONSTANT)

    /** @returns whether there's a ktp-dialout-ui executable */
    Q_PROPERTY(bool canDial READ canDial)

    /** @returns whether there's a ktp-send-file executable */
    Q_PROPERTY(bool canSendFiles READ canSendFiles)

public:
    TelepathyManager(QObject *parent=0);
    virtual ~TelepathyManager();

    /** Returns the account manager*/
    Tp::AccountManagerPtr accountManager();

    /** Add features to ObjectFactories that allow for all TextChannel features
     * This must be called before becomeReady()
     */
    Q_INVOKABLE void addTextChatFeatures();

    /** Add features to ObjectFactories that allow for all features needed for a contact list
     * This must be called before becomeReady()
     */
    Q_INVOKABLE void addContactListFeatures();

    /** Add all useful ObjectFactory features
     * This must be called before becomeReady()
     */
    Q_INVOKABLE void addAllFeatures();

    /** Call Tp::AccountManager::becomeReady
     */
    Q_INVOKABLE void becomeReady();

    /** Register an abstractClient to clientRegistrar
     * @arg client. Clients must subclass AbstractClient and QObject in their implementation to be used in QML
     *
     * @arg clientName
     * The client name MUST be a non-empty string of ASCII digits, letters, dots and/or underscores, starting with a letter, and without sets of two consecutive dots or a dot followed by a digit.
     *
     * See ClientRegistrar::registerClient for details
     *
     * @return whether registration was successful
     *
     */
    Q_INVOKABLE bool registerClient(QObject *client, const QString &clientName);

    Q_INVOKABLE bool unregisterClient(QObject* client);

    bool canDial() const;
    bool canSendFiles() const;

    /** Opens UI to start an audio call */
    Q_INVOKABLE void openDialUi() const;

    /** Opens UI to send a file */
    Q_INVOKABLE void openSendFileUi() const;

    /** Opens UI to add a new contact */
    Q_INVOKABLE void addContact();

    /** Opens UI to join a chat room */
    Q_INVOKABLE void joinChatRoom();

    /** Opens UI to show the KDE Telepathy settings module */
    Q_INVOKABLE void showSettingsKCM();

    /** Toggles the visibility of the ktp-contact-list program */
    Q_INVOKABLE void toggleContactList();

public Q_SLOTS:
    /** Start a text chat using the default KTp text application
        @arg account the account to start the channel from
        @arg contact the contact to start the channel with
        @arg delegateToPreferredHandler whether any existing handlers should release handling the channel and pass control to the requested handler
     */
    Tp::PendingChannelRequest* startChat(const Tp::AccountPtr &account,
                                         const KTp::ContactPtr &contact,
                                         bool delegateToPreferredHandler = true);

    /** Start a text chat using the preffered client
        @arg account the account to start the channel from
        @arg contact the contact to start the channel with
        @arg preferredHandler the preferred client
    */
    Tp::PendingChannelRequest* startChat(const Tp::AccountPtr &account,
                                         const KTp::ContactPtr &contact,
                                         const QString &preferredHandler);

    /** Start an audio call using the default KTp call application
        @arg account the account to start the channel from
        @arg contact the contact to start the channel with
    */
    Tp::PendingChannelRequest* startAudioCall(const Tp::AccountPtr &account,
                                              const KTp::ContactPtr &contact);

    /** Start an audio call using the default KTp call application
        @arg account the account to start the channel from
        @arg contact the contact to start the channel with
    */
    Tp::PendingChannelRequest* startAudioVideoCall(const Tp::AccountPtr &account,
                                                   const KTp::ContactPtr &contact);

    /** Start a file transfer using the default KTp file transfer application
        @arg account the account to start the channel from
        @arg contact the contact to start the channel with
    */
    Tp::PendingOperation* startFileTransfer(const Tp::AccountPtr &account,
                                            const KTp::ContactPtr &contact,
                                            const QUrl& url);

    /** Open logs using the default KTp log application
        @arg account the account to start the channel from
        @arg contact the contact to start the channel with
    */
    void openLogViewer(const Tp::AccountPtr &account, const KTp::ContactPtr &contact);

private Q_SLOTS:
    void contactlistDBusAccessed(QDBusPendingCallWatcher*);

private:
    Tp::AccountManagerPtr m_accountManager;
    Tp::ClientRegistrarPtr m_clientRegistrar;

    Tp::AccountFactoryPtr m_accountFactory;
    Tp::ContactFactoryPtr m_contactFactory;
    Tp::ConnectionFactoryPtr m_connectionFactory;
    Tp::ChannelFactoryPtr m_channelFactory;

};

#endif // DECLARATIVEKTPACTIONS_H
