/*
    Copyright (C) 2012 George Kiagiadakis <kiagiadakis.george@gmail.com>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "capabilities-hack-private.h"

/*
 * This is a hack to workaround a gabble bug.
 * https://bugs.freedesktop.org/show_bug.cgi?id=51978
 */

namespace CapabilitiesHackPrivate {

static Tp::RequestableChannelClassSpec gabbleAudioCallRCC()
{
    static Tp::RequestableChannelClassSpec spec;

    if (!spec.isValid()) {
        Tp::RequestableChannelClass rcc;
        rcc.fixedProperties.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".ChannelType"),
                TP_QT_IFACE_CHANNEL_TYPE_CALL);
        rcc.fixedProperties.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandleType"),
                (uint) Tp::HandleTypeContact);
        rcc.allowedProperties.append(TP_QT_IFACE_CHANNEL_TYPE_CALL + QLatin1String(".InitialAudio"));
        rcc.allowedProperties.append(TP_QT_IFACE_CHANNEL_TYPE_CALL + QLatin1String(".InitialAudioName"));
        spec = Tp::RequestableChannelClassSpec(rcc);
    }

    return spec;
}

static Tp::RequestableChannelClassSpec gabbleVideoCallRCC()
{
    static Tp::RequestableChannelClassSpec spec;

    if (!spec.isValid()) {
        Tp::RequestableChannelClass rcc;
        rcc.fixedProperties.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".ChannelType"),
                TP_QT_IFACE_CHANNEL_TYPE_CALL);
        rcc.fixedProperties.insert(TP_QT_IFACE_CHANNEL + QLatin1String(".TargetHandleType"),
                (uint) Tp::HandleTypeContact);
        rcc.allowedProperties.append(TP_QT_IFACE_CHANNEL_TYPE_CALL + QLatin1String(".InitialVideo"));
        rcc.allowedProperties.append(TP_QT_IFACE_CHANNEL_TYPE_CALL + QLatin1String(".InitialVideoName"));
        spec = Tp::RequestableChannelClassSpec(rcc);
    }

    return spec;
}

bool audioCalls(const Tp::CapabilitiesBase &caps, const QString &cmName)
{
    bool gabbleResult = false;
    if (cmName == QLatin1String("gabble")) {
        Q_FOREACH (const Tp::RequestableChannelClassSpec &rccSpec, caps.allClassSpecs()) {
            if (rccSpec.supports(gabbleAudioCallRCC())) {
                gabbleResult = true;
                break;
            }
        }
    }

    return gabbleResult || caps.audioCalls();
}

bool videoCalls(const Tp::CapabilitiesBase &caps, const QString &cmName)
{
    bool gabbleResult = false;
    if (cmName == QLatin1String("gabble")) {
        Q_FOREACH (const Tp::RequestableChannelClassSpec &rccSpec, caps.allClassSpecs()) {
            if (rccSpec.supports(gabbleVideoCallRCC())) {
                gabbleResult = true;
                break;
            }
        }
    }

    return gabbleResult || caps.videoCalls();
}

}
