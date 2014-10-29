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

#ifndef KTP_ACTIONS_H
#define KTP_ACTIONS_H

#include <TelepathyQt/Types>

#include <KTp/ktpcommoninternals_export.h>

namespace Tp {
class PendingChannelRequest;
class PendingOperation;
}

namespace KTp {

namespace Actions {

    typedef QList<QUrl> DocumentList;

    KTPCOMMONINTERNALS_EXPORT Tp::PendingChannelRequest* startChat(const Tp::AccountPtr &account,
                                                    const QString &contactIdentifier,
                                                    bool delegateToPreferredHandler = true);

    KTPCOMMONINTERNALS_EXPORT Tp::PendingChannelRequest* startChat(const Tp::AccountPtr &account,
                                                    const Tp::ContactPtr &contact,
                                                    bool delegateToPreferredHandler = true);

    KTPCOMMONINTERNALS_EXPORT Tp::PendingChannelRequest* startGroupChat(const Tp::AccountPtr &account,
                                                         const QString &roomName);

    KTPCOMMONINTERNALS_EXPORT Tp::PendingChannelRequest* startAudioCall(const Tp::AccountPtr &account,
                                                         const Tp::ContactPtr &contact);

    KTPCOMMONINTERNALS_EXPORT Tp::PendingChannelRequest* startAudioVideoCall(const Tp::AccountPtr &account,
                                                              const Tp::ContactPtr &contact);

    KTPCOMMONINTERNALS_EXPORT Tp::PendingChannelRequest* startDesktopSharing(const Tp::AccountPtr &account,
                                                              const Tp::ContactPtr &contact);

    KTPCOMMONINTERNALS_EXPORT Tp::PendingChannelRequest* startFileTransfer(const Tp::AccountPtr &account,
                                                            const Tp::ContactPtr &contact,
                                                            const QString &filePath);

    KTPCOMMONINTERNALS_EXPORT Tp::PendingOperation* startFileTransfer(const Tp::AccountPtr &account,
                                                       const Tp::ContactPtr &contact,
                                                       const QUrl &url);

    KTPCOMMONINTERNALS_EXPORT Tp::PendingChannelRequest* startCollaborativeEditing(const Tp::AccountPtr& account,
                                                                    const Tp::ContactPtr& contact,
                                                                    const DocumentList& documents,
                                                                    bool needOpenEditor=false);


    KTPCOMMONINTERNALS_EXPORT Tp::PendingChannelRequest* startCollaborativeEditing(const Tp::AccountPtr& account,
                                                                    const QString& chatroom,
                                                                    const DocumentList& documents,
                                                                    bool needOpenEditor=false);

    KTPCOMMONINTERNALS_EXPORT void openLogViewer(const Tp::AccountPtr &account,
                                  const Tp::ContactPtr &contact);

    KTPCOMMONINTERNALS_EXPORT void openLogViewer(const Tp::AccountPtr &account,
                                  const QString &targetId);

    KTPCOMMONINTERNALS_EXPORT void openLogViewer(const QUrl &uri);


} /* namespace Actions */

} /* namespace KTp */

#endif // KTP_ACTIONS_H
