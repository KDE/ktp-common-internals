/***************************************************************************
 *   Copyright (C) 2011 by Francesco Nwokeka <francesco.nwokeka@gmail.com> *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "telepathyContactList.h"

#include <KStandardDirs>
// #include <KUrl>
//
// #include <Nepomuk/File>
// #include <Nepomuk/Vocabulary/NIE>
// #include <Nepomuk/ResourceManager>
// #include <Nepomuk/Variant>

#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>

TelepathyContactList::TelepathyContactList(QObject* parent, const QVariantList& args)
    : Applet(parent, args)
    , m_declarative(new Plasma::DeclarativeWidget(this))
    , m_qmlObject(0)
{
    // set plasmoid size
    setMinimumSize(250, 400);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
}

TelepathyContactList::~TelepathyContactList()
{
    delete m_declarative;
}

int TelepathyContactList::appletHeight() const
{
    return geometry().height();
}

int TelepathyContactList::appletWidth() const
{
    return geometry().width();
}

// QString TelepathyContactList::extractAvatarPathFromNepomuk(const QString &nepomukUri)
// {
//     /// TODO The resource doesn't have a file url
//     /// The Telepathy Nepomuk service isn't pushing the data properly. So wait till this is done for avatar support
//     Nepomuk::Resource asd(nepomukUri.toUrl());
//     qDebug() << "REsourCE is FILE: " << asd.isFile();
//     qDebug() << "INCOMING URI is: " << nepomukUri.toString();
//     Nepomuk::File file(KUrl(nepomukUri.toString()));
//     qDebug() << "VALID: " << file.isValid();
//     qDebug() << "TEST 2 NIE: " << asd.property(Nepomuk::Vocabulary::NIE::url()).toString();
//
//     return nepomukUri.toString();
// }

void TelepathyContactList::init()
{
    // load QML part of the plasmoid
    if (m_declarative) {
        QString qmlFile = KGlobal::dirs()->findResource("data", "plasma/plasmoids/org.kde.telepathy-contact-list/contents/ui/main.qml");
        qDebug() << "LOADING: " << qmlFile;
        m_declarative->setQmlPath(qmlFile);

        // make C++ Plasma::Applet available to QML for resize signal
        m_declarative->engine()->rootContext()->setContextProperty("TelepathyContactList", this);

        // setup qml object so that we can talk to the declarative part
        m_qmlObject = dynamic_cast<QObject*>(m_declarative->rootObject());

        // connect the qml object to recieve signals from C++ end
        // these two signals are for the plasmoid resize. QML can't determine the Plasma::DeclarativeWidget's boundaries
        connect(this, SIGNAL(widthChanged()), m_qmlObject, SLOT(onWidthChanged()));
        connect(this, SIGNAL(heightChanged()), m_qmlObject, SLOT(onHeightChanged()));
    }
}


// This is the command that links your applet to the .desktop file
K_EXPORT_PLASMA_APPLET(telepathy-contact-list, TelepathyContactList)
