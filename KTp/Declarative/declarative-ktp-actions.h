/*
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

#ifndef DECLARATIVEKTPACTIONS_H
#define DECLARATIVEKTPACTIONS_H

#include <QObject>
#include <TelepathyQt/Types>
#include <KTp/contact.h>

namespace Tp { class PendingChannelRequest; }

class DeclarativeKTpActions : public QObject
{
    Q_OBJECT
    public Q_SLOTS:
        Tp::PendingChannelRequest* startChat(const Tp::AccountPtr &account,
                                                    const KTp::ContactPtr &contact,
                                                    bool delegateToPreferredHandler = true);

        Tp::PendingChannelRequest* startAudioCall(const Tp::AccountPtr &account,
                                                            const KTp::ContactPtr &contact);

        Tp::PendingChannelRequest* startAudioVideoCall(const Tp::AccountPtr &account,
                                                                const KTp::ContactPtr &contact);

        Tp::PendingChannelRequest* startDesktopSharing(const Tp::AccountPtr &account,
                                                                const KTp::ContactPtr &contact);

        Tp::PendingOperation* startFileTransfer(const Tp::AccountPtr &account,
                                                const KTp::ContactPtr &contact,
                                                const QUrl& url);

        Tp::PendingChannelRequest* startFileTransfer(const Tp::AccountPtr &account,
                                                                const KTp::ContactPtr &contact,
                                                                const QString &filePath);

        void openLogViewer(const Tp::AccountPtr &account, const KTp::ContactPtr &contact);
};

#endif // DECLARATIVEKTPACTIONS_H
