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

#include "declarative-ktp-actions.h"
#include <KTp/actions.h>

void DeclarativeKTpActions::openLogViewer(const Tp::AccountPtr &account, const KTp::ContactPtr &contact)
{
    KTp::Actions::openLogViewer(account, contact);
}

Tp::PendingChannelRequest* DeclarativeKTpActions::startAudioCall(const Tp::AccountPtr &account, const KTp::ContactPtr &contact)
{
    return KTp::Actions::startAudioCall(account, contact);
}

Tp::PendingChannelRequest* DeclarativeKTpActions::startAudioVideoCall(const Tp::AccountPtr &account, const KTp::ContactPtr &contact)
{
    return KTp::Actions::startAudioVideoCall(account, contact);
}

Tp::PendingChannelRequest* DeclarativeKTpActions::startChat(const Tp::AccountPtr &account, const KTp::ContactPtr &contact, bool delegateToPreferredHandler)
{
    return KTp::Actions::startChat(account, contact, delegateToPreferredHandler);
}

Tp::PendingChannelRequest* DeclarativeKTpActions::startDesktopSharing(const Tp::AccountPtr &account, const KTp::ContactPtr &contact)
{
    return KTp::Actions::startDesktopSharing(account, contact);
}

Tp::PendingChannelRequest* DeclarativeKTpActions::startFileTransfer(const Tp::AccountPtr &account, const KTp::ContactPtr &contact, const QString &filePath)
{
    return KTp::Actions::startFileTransfer(account, contact, filePath);
}

Tp::PendingOperation* DeclarativeKTpActions::startFileTransfer(const Tp::AccountPtr &account, const KTp::ContactPtr &contact, const QUrl &url)
{
    return KTp::Actions::startFileTransfer(account, contact, url);
}
