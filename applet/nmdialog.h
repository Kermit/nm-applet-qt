#ifndef NMDIALOG_H
#define NMDIALOG_H

#include <QtGui/QDialog>
#include "connectionslistmodel.h"
#include "interfaceslistmodel.h"
#include "declarativeinterfaceitem.h"
#include "listitemdelegate.h"

namespace Ui {
class NMDialog;
}

class NMDialog : public QDialog
{
        Q_OBJECT
    public:
        explicit NMDialog(RemoteActivatableList * activatableList, QWidget *parent = 0);
        ~NMDialog();
        void updateHasWireless(bool checked = true);
        void updateHasWwan();
        void addVpnInterface();
        DeclarativeInterfaceItem * vpnItem();

        QHash<QString, DeclarativeInterfaceItem*> m_interfaces;

    public Q_SLOTS:
        void updateWireless(bool checked);
        void updateMobile(bool checked);
        void manageConnections();
        void showInterfaceDetails(const QString & uni);
        void manageSelection();
        void countConnectionsList();


    private Q_SLOTS:
        void readConfig();
        void managerNetworkingEnabledChanged(bool);
        void networkingEnabledToggled(bool);
        void managerWirelessEnabledChanged(bool);
        void managerWirelessHardwareEnabledChanged(bool);
        void managerWwanHardwareEnabledChanged(bool);
        void managerWwanEnabledChanged(bool);
        void interfaceRemoved(const QString& uni);
        void interfaceAdded(const QString& uni);
        void manageUpdateTraffic(DeclarativeInterfaceItem *device);

        void createActions();
    private:
        void addInterfaceInternal(const NetworkManager::Device::Ptr &);
        ConnectionsListModel *listModel;
        ListItemDelegate *listItemDelegate;
        InterfacesListModel *interfaceListModel;
        DeclarativeInterfaceItem *m_vpnItem;
        bool m_hasWirelessInterface;
        RemoteActivatableList* m_activatables;
        QMap<QString, QAction *> m_actions;
        Ui::NMDialog *ui;
};

#endif // NMDIALOG_H
