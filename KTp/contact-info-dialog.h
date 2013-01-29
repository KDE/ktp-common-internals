/*
 * Copyright (C) 2013 Dan Vrátil <dvratil@redhat.com>
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

#ifndef KTP_CONTACTINFODIALOG_H
#define KTP_CONTACTINFODIALOG_H

#include <KDialog>

#include <KTp/ktp-export.h>

#include <TelepathyQt/Types>

namespace Tp {
class PendingOperation;
}

namespace KTp {

class KTP_EXPORT ContactInfoDialog : public KDialog
{

    Q_OBJECT

  public:
    explicit ContactInfoDialog(const Tp::AccountPtr &account, const Tp::ContactPtr &contact, QWidget* parent = 0);
    virtual ~ContactInfoDialog();

  protected:
    virtual void slotButtonClicked(int button);

  private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void onContactUpgraded(Tp::PendingOperation*))
    Q_PRIVATE_SLOT(d, void onContactInfoReceived(Tp::PendingOperation*))
    Q_PRIVATE_SLOT(d, void onChangeAvatarButtonClicked())
    Q_PRIVATE_SLOT(d, void onClearAvatarButtonClicked())
    Q_PRIVATE_SLOT(d, void onInfoDataChanged())

};

} // namespace KTp

#endif // KTP_CONTACTINFODIALOG_H
