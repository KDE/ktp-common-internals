/*
* Copyright (C) 2012 David Edmundson <kde@davidedmundson.co.uk>
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

#ifndef KTP_CONTACT_H
#define KTP_CONTACT_H

#include <QVariant>
#include <TelepathyQt/Contact>
#include <KTp/presence.h>
#include <KTp/ktp-export.h>


namespace KTp{
class KTP_EXPORT Contact : public Tp::Contact
{
    Q_OBJECT
public:
    explicit Contact(Tp::ContactManager *manager, const Tp::ReferencedHandles &handle, const Tp::Features &requestedFeatures, const QVariantMap &attributes);

    KTp::Presence presence() const;


     /** Returns true if audio calls can be started with this contact*/
     bool audioCallCapability() const;
     /** Returns true if video calls can be started with this contact*/
     bool videoCallCapability() const;
     /** Returns true if file transfers can be started with this contact*/
     bool fileTransferCapability() const;

     //Overridden as a workaround for upstream bug https://bugs.freedesktop.org/show_bug.cgi?id=55883
     QStringList clientTypes() const;
     /** Returns the pixmap of an avatar coloured to gray if contact online*/
     QPixmap avatarPixmap();

Q_SIGNALS:
    void invalidated();

private Q_SLOTS:
    void invalidateAvatarCache();

private:
    void avatarToGray(QPixmap &avatar);
    QString keyCache() const;
    QString buildAvatarPath(const QString &avatarToken);

};


typedef Tp::SharedPtr<KTp::Contact> ContactPtr;

}//namespace

// Q_DECLARE_METATYPE(KTp::ContactPtr)

#endif // CONTACT_H
