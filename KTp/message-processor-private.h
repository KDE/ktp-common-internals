/*
    Copyright (C) 2013 Daniel Vrátil <dvratil@redhat.com>

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

#include "message-processor.h"

using namespace KTp;

class FilterPlugin
{
  public:
    explicit FilterPlugin(const KPluginInfo &pluginInfo, KTp::AbstractMessageFilter *instance);
    explicit FilterPlugin(const QString &name,  int weight, KTp::AbstractMessageFilter *instance);

    bool operator<(const FilterPlugin &other) const;
    bool operator==(const FilterPlugin &other) const;

    QString name;
    int weight;
    KTp::AbstractMessageFilter* instance;
};

class MessageProcessor::Private
{
  public:
    Private(MessageProcessor *parent):
        q(parent)
    { }

    void loadFilters();

    void loadFilter(const KPluginInfo &pluginInfo);
    void unloadFilter(const KPluginInfo &pluginInfo);

    QList<FilterPlugin> filters;

  private:
    MessageProcessor *q;
};
