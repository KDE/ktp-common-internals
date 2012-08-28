/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Vishesh Handa <me@vhanda.in>

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


#include "nepomukfeeder.h"

#include <Nepomuk2/ResourceManager>

#include <Soprano/Model>
#include <Soprano/StatementIterator>
#include <Soprano/NodeIterator>
#include <KDebug>

void NepomukFeeder::setGraph(const Nepomuk2::SimpleResourceGraph& graph)
{
    m_graph = graph;
}

namespace {
    QUrl fetchGraph( const QUrl& uri, const QUrl& prop = QUrl() ) {
        Soprano::Model* model = Nepomuk2::ResourceManager::instance()->mainModel();
        QList< Soprano::Node > list = model->listStatements( uri, prop, QUrl() ).iterateContexts().allNodes();
        if( list.isEmpty() )
            return QUrl();

        return list.first().uri();
    }

    Soprano::Node variantToNode( const QVariant& var ) {
        if( var.type() == QVariant::Url )
            return var.toUrl();
        else {
            Soprano::LiteralValue lv( var );
            return Soprano::Node( lv );
        }
    }
}

// TODO: Move to the DMS
void NepomukFeeder::push()
{
    Soprano::Model* model = Nepomuk2::ResourceManager::instance()->mainModel();

    QList<Nepomuk2::SimpleResource> resList  = m_graph.toList();
    foreach( const Nepomuk2::SimpleResource& res, resList ) {
        const QUrl uri = res.uri();

        QUrl lastGraph = fetchGraph( uri );

        const Nepomuk2::PropertyHash& propHash = res.properties();
        const QList<QUrl> keys = propHash.uniqueKeys();
        foreach( const QUrl& prop, keys ) {
            QList<QVariant> values = propHash.values( prop );

            QUrl graph = fetchGraph( uri, prop );
            if( graph.isEmpty() )
                graph = lastGraph;

            // This might leave some orphan graphs
            model->removeAllStatements( uri, prop, QUrl() );
            foreach( const QVariant& var, values ) {
                if( var.isNull() ) {
                    continue;
                }
                Soprano::Statement st( uri, prop, variantToNode(var), graph );
                kDebug() << st;
                model->addStatement( st );
            }

            lastGraph = graph;
        }
    }
}

