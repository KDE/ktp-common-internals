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

#include "logs-importer-private.h"
#include "logs-importer.h"

#include <KLocalizedString>
#include <KDebug>
#include <KStandardDirs>

using namespace KTp;

LogsImporter::Private::Private(KTp::LogsImporter* parent)
  : QThread(parent)
  , m_day(0)
  , m_month(0)
  , m_year(0)
  , m_isMUCLog(false)
{

}

LogsImporter::Private::~Private()
{

}

void LogsImporter::Private::setAccountId(const QString& accountId)
{
    m_accountId = accountId;
}

void LogsImporter::Private::run()
{
    QStringList files = findKopeteLogs(m_accountId);
    if (files.isEmpty()) {
        Q_EMIT error(i18n("No Kopete logs found"));
        return;
    }

    Q_FOREACH (const QString &file, files) {
        convertKopeteLog(file);
    }
}

QString LogsImporter::Private::accountIdToAccountName(const QString &accountId) const
{
    int plugin = accountId.indexOf(QLatin1Char('/'));
    int protocol = accountId.indexOf(QLatin1Char('/'), plugin + 1);

    QString username = accountId.mid(protocol + 1);

    /* ICQ accounts are prefixed with '_X' (X being a number) */
    if (username.startsWith(QLatin1Char('_'))) {
        username = username.remove(0, 2);
    }

    /* Remove trailing "0" */
    username.chop(1);

    /* Kopete escapes ".", "/", "~", "?" and "*" as "-" */
    username.replace(QLatin1String("_2e"), QLatin1String("-")); /* . */
    username.replace(QLatin1String("_2f"), QLatin1String("-")); /* / */
    username.replace(QLatin1String("_7e"), QLatin1String("-")); /* ~ */
    username.replace(QLatin1String("_3f"), QLatin1String("-")); /* ? */
    username.replace(QLatin1String("_2a"), QLatin1String("-")); /* * */

    /* But Kopete has apparently no problem with "@", so unescape it */
    username.replace(QLatin1String("_40"), QLatin1String("@"));

    return username;
}

QString LogsImporter::Private::accountIdToProtocol(const QString &accountId) const
{
    if (accountId.startsWith(QLatin1String("haze/aim/"))) {
        return QLatin1String("AIMProtocol");
    } else if (accountId.startsWith(QLatin1String("haze/msn/"))) {
        return QLatin1String("WlmProtocol");
    } else if (accountId.startsWith(QLatin1String("haze/icq/"))) {
        return QLatin1String("ICQProtocol");
    } else if (accountId.startsWith(QLatin1String("haze/yahoo/"))) {
        return QLatin1String("YahooProtocol");
    } else if (accountId.startsWith(QLatin1String("gabble/jabber/"))) {
        return QLatin1String("JabberProtocol");
    } else if (accountId.startsWith(QLatin1String("sunshine/gadugadu/")) ||
               accountId.startsWith(QLatin1String("haze/gadugadu/"))) {
        return QLatin1String("GaduProtocol");
    } else {
        /* We don't support these Kopete protocols:
         *         Bonjour - unable to reliably map Telepathy account to Kopete
         *         GroupWise - no support in Telepathy
         *         Meanwhile - no support in Telepathy
         *         QQ - no support in Telepathy
         *         SMS - no support in Telepathy
         *         Skype - not supported by KTp
         *         WinPopup - no support in Telepathy
         */
        kWarning() << accountId << "is an unsupported protocol";
        return QString();
    }
}

QStringList LogsImporter::Private::findKopeteLogs(const QString &accountId) const
{
    QStringList files;

    QString protocol = accountIdToProtocol(accountId);
    if (protocol.isEmpty()) {
        kWarning() << "Unsupported protocol";
        return files;
    }

    QString kopeteAccountId = accountIdToAccountName(accountId);
    if (kopeteAccountId.isEmpty()) {
        kWarning() << "Unable to parse account ID";
        return files;
    }

    QDir dir(KStandardDirs::locateLocal("data", QLatin1String("kopete/logs/") +
             protocol + QDir::separator() + kopeteAccountId));

    if (dir.exists()) {
        QFileInfoList entries = dir.entryInfoList(QStringList() << QLatin1String("*.xml"), QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
        Q_FOREACH (const QFileInfo &finfo, entries) {
            files << finfo.filePath();
        }
    }

    return files;
}

void LogsImporter::Private::initKTpDocument()
{
    m_ktpDocument.clear();
    m_ktpLogElement.clear();

    QDomNode xmlNode = m_ktpDocument.createProcessingInstruction(
        QLatin1String("xml"), QLatin1String("version='1.0' encoding='utf-8'"));
    m_ktpDocument.appendChild(xmlNode);

    xmlNode = m_ktpDocument.createProcessingInstruction(
        QLatin1String("xml-stylesheet"), QLatin1String("type=\"text/xsl\" href=\"log-store-xml.xsl\""));
    m_ktpDocument.appendChild(xmlNode);

    m_ktpLogElement = m_ktpDocument.createElement(QLatin1String("log"));
    m_ktpDocument.appendChild(m_ktpLogElement);
}

void LogsImporter::Private::saveKTpDocument()
{
    QString filename = QString(QLatin1String("%1%2%3.log"))
        .arg(m_year)
        .arg(m_month, 2, 10, QLatin1Char('0'))
        .arg(m_day, 2, 10, QLatin1Char('0'));

    KStandardDirs dirs;
    QString path = dirs.localxdgdatadir() + QDir::separator() + QLatin1String("TpLogger") + QDir::separator() + QLatin1String("logs");

    if (m_isMUCLog) {
        path += QDir::separator() + QLatin1String("chatrooms");
    } else {
        QString accountId = m_accountId;
        /* Escape '/' in accountId as '_' */
        if (m_accountId.contains(QLatin1Char('/'))) {
          accountId.replace(QLatin1Char('/'), QLatin1String("_"));
        }
        path += QDir::separator() + accountId;
    }

    path += QDir::separator() + m_contactId;

    /* Make sure the path exists */
    QDir dir(path);
    if (!dir.exists()) {
        QDir::home().mkpath(QDir::home().relativeFilePath(dir.path()));
    }

    path += QDir::separator() + filename;

    QFile outFile(path);
    if (outFile.exists()) {
        kWarning() << path << "already exists, not importing logs";
        return;
    }

    outFile.open(QIODevice::WriteOnly);
    QTextStream stream(&outFile);
    m_ktpDocument.save(stream, 0);

    kDebug() << "Stored as" << path;
}

KDateTime LogsImporter::Private::parseKopeteTime(const QDomElement& kopeteMessage) const
{
    QString strtime = kopeteMessage.attribute(QLatin1String("time"));
    if (strtime.isEmpty()) {
        return KDateTime();
    }

    /* Kopete time attribute is in format "D H:M:S" - year and month are stored in
     * log header, Hour, minute and seconds don't have zero padding */
    QStringList dateTime = strtime.split(QLatin1Char(' '), QString::SkipEmptyParts);
    if (dateTime.length() != 2) {
        return KDateTime();
    }

    QStringList time = dateTime.at(1).split(QLatin1Char(':'));

    QString str = QString(QLatin1String("%1-%2-%3T%4:%5:%6Z"))
        .arg(m_year)
        .arg(m_month, 2, 10, QLatin1Char('0'))
        .arg(dateTime.at(0).toInt(), 2, 10, QLatin1Char('0'))
        .arg(time.at(0).toInt(), 2, 10, QLatin1Char('0'))
        .arg(time.at(1).toInt(), 2, 10, QLatin1Char('0'))
        .arg(time.at(2).toInt(), 2, 10, QLatin1Char('0'));

    /* Kopete stores date in local timezone but Telepathy in UTC. Note that we
     * must use time offset at the specific date rather then current offset
     * (could be different due to for example DST) */
    KDateTime localTz = KDateTime::fromString(str, KDateTime::ISODate);
    KDateTime utc = localTz.addSecs(-KDateTime::currentLocalDateTime().timeZone().offset(localTz.toTime_t()));

    return utc;
}

QDomElement LogsImporter::Private::convertKopeteMessage(const QDomElement& kopeteMessage)
{
    KDateTime time = parseKopeteTime(kopeteMessage);
    if (!time.isValid()) {
        kWarning() << "Failed to parse message time, skipping message";
        return QDomElement();
    }

    /* If this is the very first message we are processing, then initialize
    * the day counter */
    if (m_day == 0) {
        m_day = time.date().day();
    }

    /* Kopete stores logs by months, while Telepathy by days. When day changes,
    * save to current KTp log and prepare a new document */
    if (time.date().day() != m_day) {
        saveKTpDocument();
        m_day = time.date().day();

        initKTpDocument();
    }

    QDomElement ktpMessage = m_ktpDocument.createElement(QLatin1String("message"));
    ktpMessage.setAttribute(QLatin1String("time"), time.toUtc().toString(QLatin1String("%Y%m%dT%H:%M:%S")));

    QString sender = kopeteMessage.attribute(QLatin1String("from"));
    if (!m_isMUCLog && sender.startsWith(m_contactId) && sender.length() > m_contactId.length()) {
        m_isMUCLog = true;
    }

    /* In MUC, the "from" attribute is in format "room@conf.server/senderId", so strip
     * the room name */
    if (m_isMUCLog) {
        sender = sender.remove(m_contactId);
    }

    ktpMessage.setAttribute(QLatin1String("id"), sender);
    ktpMessage.setAttribute(QLatin1String("name"), kopeteMessage.attribute(QLatin1String("nick")));

    if (sender == m_meId) {
        ktpMessage.setAttribute(QLatin1String("isuser"), QLatin1String("true"));
    } else {
        ktpMessage.setAttribute(QLatin1String("isuser"), QLatin1String("false"));
    }

    /* These are not present in Kopete logs, but that should not matter */
    ktpMessage.setAttribute(QLatin1String("token"), QString());
    ktpMessage.setAttribute(QLatin1String("message-token"), QString());
    ktpMessage.setAttribute(QLatin1String("type"), QLatin1String("normal"));

    /* Copy the message content */
    QDomText message = m_ktpDocument.createTextNode(kopeteMessage.text());
    ktpMessage.appendChild(message);

    return ktpMessage;
}

void LogsImporter::Private::convertKopeteLog(const QString& filepath)
{
    kDebug() << "Converting" << filepath;

    /* Init */
    m_day = 0;
    m_month = 0;
    m_year = 0;
    m_isMUCLog = false;
    m_meId.clear();
    m_contactId.clear();

    initKTpDocument();

    QFile f(filepath);
    f.open(QIODevice::ReadOnly);

    QByteArray ba = f.readAll();

    m_kopeteDocument.setContent(ba);
    /* Get <history> node */
    QDomElement history = m_kopeteDocument.documentElement();
    /* Get all <msg> nodes in <history> node */
    QDomNodeList kopeteMessages = history.elementsByTagName(QLatin1String("msg"));

    /* Get <head> node and parse it */
    QDomNodeList heads = history.elementsByTagName(QLatin1String("head"));
    if (heads.isEmpty()) {
        Q_EMIT error(i18n("Invalid Kopete log format"));
        return;
    }

    QDomNode head = heads.item(0);
    QDomNodeList headData = head.childNodes();
    if (headData.length() < 3) {
        Q_EMIT error(i18n("Invalid Kopete log format"));
        return;
    }

    for (int i = 0; i < headData.count(); i++) {
        QDomElement el = headData.item(i).toElement();

        if (el.tagName() == QLatin1String("date")) {
            m_year = el.attribute(QLatin1String("year"), QString()).toInt();
            m_month = el.attribute(QLatin1String("month"), QString()).toInt();
        } else if (el.tagName() == QLatin1String("contact")) {
            if (el.attribute(QLatin1String("type")) == QLatin1String("myself")) {
                m_meId = el.attribute(QLatin1String("contactId"));
            } else {
                m_contactId = el.attribute(QLatin1String("contactId"));
            }
        }
    }

    if ((m_year == 0) || (m_month == 0) || m_meId.isEmpty() || m_contactId.isEmpty()) {
        kWarning() << "Failed to correctly parse header. Possibly invalid log format";
        return;
    }

    for (int i = 0; i < kopeteMessages.count(); i++) {
        QDomElement kopeteMessage = kopeteMessages.item(i).toElement();

        QDomElement ktpMessage = convertKopeteMessage(kopeteMessage);

        m_ktpLogElement.appendChild(ktpMessage);
    }

    saveKTpDocument();
}
