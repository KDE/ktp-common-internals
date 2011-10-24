/*
    Telepathy error dictionary - usable strings for dbus messages
    Copyright (C) 2011  Martin Klapetek <martin.klapetek@gmail.com>

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


#ifndef ERROR_DICTIONARY_H
#define ERROR_DICTIONARY_H

#include <QObject>
#include <QHash>

class ErrorDictionary : public QObject
{

public:
    static ErrorDictionary *instance();
    virtual ~ErrorDictionary();

    ///Returns an error message usable for displaying to the user
    QString displayErrorMessage(const QString& dbusErrorName) const;

private:
    ErrorDictionary(QObject *parent);
    QHash<QString, QString> m_dict;

    static ErrorDictionary *m_instance;
};

#endif // ERROR_DICTIONARY_H
