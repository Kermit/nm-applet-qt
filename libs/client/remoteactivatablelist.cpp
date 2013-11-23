/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2011-2012 Lamarque V. Souza <lamarque@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "remoteactivatablelist.h"

#include <QtCore/QHash>
#include <QtCore/QStringList>
#include <QtDBus/QDBusInterface>

#include <QtCore/QDebug>

#include "networkmanagementinterface.h"

#include "remoteactivatable.h"
#include "remoteinterfaceconnection.h"
#include "remoteunconfiguredinterface.h"
#include "remotevpninterfaceconnection.h"
#include "remotewirelessinterfaceconnection.h"
#include "remotewirelessnetwork.h"
#include "remotegsminterfaceconnection.h"

class RemoteActivatableListPrivate
{
    public:
        NetworkManagementInterface *iface;
        QHash<QString, RemoteActivatable *> activatables;
        QList<RemoteActivatable *> sortedActivatables;
        QDBusServiceWatcher *registrationWatcher;
};


RemoteActivatableList::RemoteActivatableList(QObject *parent)
    : QObject(parent), d_ptr(new RemoteActivatableListPrivate)
{
    Q_D(RemoteActivatableList);
    d->iface = new NetworkManagementInterface("org.kde.networkmanagement", "/org/kde/networkmanagement", QDBusConnection::sessionBus(), this);
    // clean our connections out if the service goes away
    d->registrationWatcher = new QDBusServiceWatcher(d->iface->service(),
            QDBusConnection::sessionBus(),
            QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
            this);
    connect(d->registrationWatcher, SIGNAL(serviceRegistered(QString)), SLOT(serviceRegistered()));
    connect(d->registrationWatcher, SIGNAL(serviceUnregistered(QString)), SLOT(serviceUnregistered()));
    qDebug() << "Constructor RemoteActivatableList";
}

void RemoteActivatableList::init()
{
    Q_D(RemoteActivatableList);
    if (d->iface->isValid())
    {
        connect(d->iface, SIGNAL(ActivatableAdded(QVariantMap)),
                this, SLOT(handleActivatableAdded(QVariantMap)));
        connect(d->iface, SIGNAL(ActivatableRemoved(QString)),
                this, SLOT(handleActivatableRemoved(QString)));

        if (d->activatables.isEmpty())
        {
            d->iface->ReEmitActivatableList();
        }
        qDebug() << "Init remote activatable list" << d->activatables.count();
    }
}

bool RemoteActivatableList::isConnectionForInterface(RemoteActivatable *activatable, const NetworkManager::Device::Ptr &interface)
{
    if (activatable->deviceUni() == interface->uni())
    {
        RemoteInterfaceConnection *remoteconnection = qobject_cast<RemoteInterfaceConnection *>(activatable);
        if (remoteconnection)
        {
            if (remoteconnection->activationState() == Knm::InterfaceConnection::Activated ||
                    remoteconnection->activationState() == Knm::InterfaceConnection::Activating)
            {
                return true;
            }
        }
    }
    return false;
}


RemoteInterfaceConnection *RemoteActivatableList::connectionForInterface(const NetworkManager::Device::Ptr &interface)
{
    foreach (RemoteActivatable * activatable, activatables())
    {
        if (isConnectionForInterface(activatable, interface))
        {
            return qobject_cast<RemoteInterfaceConnection *>(activatable);
        }
    }
    return 0;
}

void RemoteActivatableList::clear()
{
    Q_D(RemoteActivatableList);
    QHash<QString, RemoteActivatable *>::iterator i = d->activatables.begin();
    while (i != d->activatables.end())
    {
        RemoteActivatable *activatable = i.value();
        d->sortedActivatables.removeOne(activatable);
        i = d->activatables.erase(i);
        emit activatableRemoved(activatable);
        // Hacky, I know, but even with deleteLater sometimes we get dangling pointers,
        // so give more time for the emit above be processed before we delete the object.
        // TODO: make sure all activatableRemoved signals were processed before deleting
        // the object.
        QTimer::singleShot(10000, activatable, SLOT(deleteLater()));
    }
}

RemoteActivatableList::~RemoteActivatableList()
{
    delete d_ptr;
}

QList<RemoteActivatable *> RemoteActivatableList::activatables() const
{
    Q_D(const RemoteActivatableList);
    return d->sortedActivatables;
}

void RemoteActivatableList::handleActivatableAdded(const QVariantMap &properties)
{
    qDebug() << "RemoteActivatableList::handleActivatableAdded(const QVariantMap &properties)";
    qDebug() << properties;

    QString addedPath = properties["path"].toString();
    uint type = properties["activatableType"].toUInt();
    int index = properties["activatableIndex"].toInt();

    if (!addedPath.startsWith('/'))
    {
        qDebug() << "Invalid path:" << addedPath << type;
        return;
    }
    Q_D(RemoteActivatableList);
    if (!d->activatables.contains(addedPath))
    {
        RemoteActivatable *newActivatable = 0;
        switch (type)
        {
            case Knm::Activatable::InterfaceConnection:
                newActivatable = new RemoteInterfaceConnection(properties, this);
                qDebug() << "interfaceconnection at" << addedPath << "with type" << newActivatable->activatableType();
                break;
            case Knm::Activatable::WirelessInterfaceConnection:
                newActivatable = new RemoteWirelessInterfaceConnection(properties, this);
                qDebug() << "wirelessconnection at" << addedPath << "with type" << newActivatable->activatableType();
                break;
            case Knm::Activatable::WirelessNetwork:
                newActivatable = new RemoteWirelessNetwork(properties, this);
                qDebug() << "wirelessnetwork at" << addedPath << "with type" << newActivatable->activatableType();
                break;
            case Knm::Activatable::UnconfiguredInterface:
                newActivatable = new RemoteUnconfiguredInterface(properties, this);
                qDebug() << "unconfiguredinterface at" << addedPath << "with type" << newActivatable->activatableType();
                break;
            case Knm::Activatable::VpnInterfaceConnection:
                newActivatable = new RemoteVpnInterfaceConnection(properties, this);
                qDebug() << "vpnconnection at" << addedPath << "with type" << newActivatable->activatableType();
                break;
            case Knm::Activatable::GsmInterfaceConnection:
                newActivatable = new RemoteGsmInterfaceConnection(properties, this);
                qDebug() << "gsminterfaceconnection at" << addedPath << "with type" << newActivatable->activatableType();
                break;
        }
        if (newActivatable)
        {
            qDebug() << "RemoteActivatable Added " << addedPath;
            d->activatables.insert(addedPath, newActivatable);
            d->sortedActivatables.insert(index, newActivatable);
            emit activatableAdded(newActivatable, index);
        }
    }
}

void RemoteActivatableList::handleActivatableRemoved(const QString &removed)
{
    Q_D(RemoteActivatableList);
    qDebug() << "removed" << removed;
    RemoteActivatable *removedActivatable = d->activatables.take(removed);
    if (removedActivatable)
    {
        d->sortedActivatables.removeOne(removedActivatable);
        emit activatableRemoved(removedActivatable);

        // Hacky, I know, but even with deleteLater sometimes we get dangling pointers,
        // so give more time for the emit above be processed before we delete the object.
        // TODO: make sure all activatableRemoved signals were processed before deleting
        // the object.
        QTimer::singleShot(10000, removedActivatable, SLOT(deleteLater()));
    }
}

void RemoteActivatableList::serviceRegistered()
{
    Q_D(RemoteActivatableList);
    if (d->iface)
    {
        // this object is probably invalid now (d->iface->isValid() == false), so delete it.
        d->iface->deleteLater();
    }
    d->iface = new NetworkManagementInterface("org.kde.networkmanagement", "/org/kde/networkmanagement", QDBusConnection::sessionBus(), this);
    init();
    emit appeared();
}

void RemoteActivatableList::serviceUnregistered()
{
    clear();
    emit disappeared();
}
// vim: sw=4 sts=4 et tw=100
