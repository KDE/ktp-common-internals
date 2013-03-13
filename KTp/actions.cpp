/*
 * Copyright (C) 2012 Dan Vrátil <dvratil@redhat.com>
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

#include "actions.h"

#include <TelepathyQt/Account>
#include <TelepathyQt/ChannelRequestHints>
#include <TelepathyQt/Contact>
#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/PendingFailure>

#include <KMimeType>
#include <KToolInvocation>
#include <KDebug>
#include <KLocalizedString>

#define PREFERRED_TEXT_CHAT_HANDLER QLatin1String("org.freedesktop.Telepathy.Client.KTp.TextUi")
#define PREFERRED_FILE_TRANSFER_HANDLER QLatin1String("org.freedesktop.Telepathy.Client.KTp.FileTransfer")
#define PREFERRED_AUDIO_VIDEO_HANDLER QLatin1String("org.freedesktop.Telepathy.Client.KTp.CallUi")
#define PREFERRED_RFB_HANDLER QLatin1String("org.freedesktop.Telepathy.Client.krfb_rfb_handler")

using namespace KTp;

Tp::PendingChannelRequest* Actions::startChat(const Tp::AccountPtr &account,
                                              const Tp::ContactPtr &contact,
                                              bool delegateToPreferredHandler)
{
    if (account.isNull() || contact.isNull()) {
        kWarning() << "Parameters invalid";
    }

    kDebug() << "Requesting text channel for" << contact->id();

    Tp::ChannelRequestHints hints;
    if (delegateToPreferredHandler) {
      hints.setHint(QLatin1String("org.freedesktop.Telepathy.ChannelRequest"),
                    QLatin1String("DelegateToPreferredHandler"),
                    QVariant(true));
    }

    return account->ensureTextChat(contact,
                                   QDateTime::currentDateTime(),
                                   PREFERRED_TEXT_CHAT_HANDLER,
                                   hints);
}

Tp::PendingChannelRequest* Actions::startGroupChat(const Tp::AccountPtr &account,
                                                   const QString &roomName)
{
    if (account.isNull() || roomName.isEmpty()) {
        kWarning() << "Parameters invalid";
    }

    kDebug() << "Requesting text chat room " << roomName;

    Tp::ChannelRequestHints hints;
    hints.setHint(QLatin1String("org.kde.telepathy"), QLatin1String("forceRaiseWindow"), QVariant(true));

    return account->ensureTextChatroom(roomName,
                                       QDateTime::currentDateTime(),
                                       PREFERRED_TEXT_CHAT_HANDLER,
                                       hints);
}

Tp::PendingChannelRequest* Actions::startAudioCall(const Tp::AccountPtr &account,
                                                   const Tp::ContactPtr &contact)
{
    if (account.isNull() || contact.isNull()) {
        kWarning() << "Parameters invalid";
    }

    kDebug() << "Requesting audio channel for" << contact->id();

    return account->ensureAudioCall(contact,
                                    QLatin1String("audio"),
                                    QDateTime::currentDateTime(),
                                    PREFERRED_AUDIO_VIDEO_HANDLER);
}

Tp::PendingChannelRequest* Actions::startAudioVideoCall(const Tp::AccountPtr &account,
                                                        const Tp::ContactPtr &contact)
{
    if (account.isNull() || contact.isNull()) {
        kWarning() << "Parameters invalid";
    }

    kDebug() << "Requesting audio-video channel for" << contact->id();

    return account->ensureAudioVideoCall(contact,
                                         QLatin1String("audio"),
                                         QLatin1String("video"),
                                         QDateTime::currentDateTime(),
                                         PREFERRED_AUDIO_VIDEO_HANDLER);
}

Tp::PendingChannelRequest* Actions::startDesktopSharing(const Tp::AccountPtr &account,
                                                        const Tp::ContactPtr &contact)
{
    if (account.isNull() || contact.isNull()) {
        kWarning() << "Parameters invalid";
    }

    kDebug() << "Requesting stream tube for" << contact->id();

    return account->createStreamTube(contact,
                                     QLatin1String("rfb"),
                                     QDateTime::currentDateTime(),
                                     PREFERRED_RFB_HANDLER);
}

Tp::PendingChannelRequest* Actions::startFileTransfer(const Tp::AccountPtr &account,
                                                      const Tp::ContactPtr &contact,
                                                      const QString &filePath)
{
    if (account.isNull() || contact.isNull()) {
        kWarning() << "Parameters invalid";
    }

    kDebug() << "Requesting file transfer of" << filePath << "to" << contact->id();

    Tp::FileTransferChannelCreationProperties fileTransferProperties(
                  filePath, KMimeType::findByFileContent(filePath)->name());

    return account->createFileTransfer(contact,
                                       fileTransferProperties,
                                       QDateTime::currentDateTime(),
                                       PREFERRED_FILE_TRANSFER_HANDLER);
}

Tp::PendingOperation* Actions::startFileTransfer(const Tp::AccountPtr &account,
                                                 const Tp::ContactPtr &contact,
                                                 const QUrl &url)
{
    if (account.isNull() || contact.isNull() || url.isEmpty()) {
        kWarning() << "Parameters invalid";
    }

    kDebug() << "Requesting file transfer of" << url.toLocalFile() << "to" << contact->id();

    Tp::PendingOperation *ret = 0;
    if (url.isLocalFile()) {
        ret = startFileTransfer(account, contact, url.toLocalFile());
    } else {
        ret = new Tp::PendingFailure(QLatin1String("Failed file transfer"), QString(QLatin1String("You are only supposed to send local files, not %1")).arg(url.toString()), account);
    }
    return ret;
}

void Actions::openLogViewer(const Tp::AccountPtr &account,
                            const Tp::ContactPtr &contact)
{
    if (account.isNull() || contact.isNull()) {
        kWarning() << "Parameters invalid";
    }

    kDebug() << "Opening logviewer for" << contact->id();

    QStringList arguments;
    arguments << QLatin1String("--") << account->uniqueIdentifier() << contact->id();

    /* Use "--" so that KCmdLineArgs does not parse UIDs starting with "-" as arguments */
    KToolInvocation::kdeinitExec(QLatin1String("ktp-log-viewer"), arguments);
}
