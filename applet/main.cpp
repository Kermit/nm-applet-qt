/***************************************************************************
 *   Copyright (C) 2012 by Francisco Ballina Sanchez                       *
 *   zballinita@gmail.com                                                  *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QtGui/QApplication>
#include <QtCore/QSettings>
#include <QtGui/QMessageBox>
#include <QtGui/QSystemTrayIcon>
#include "networkmanager.h"

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(resources);

    QApplication::setApplicationName("nm-applet-qt");
#ifdef STR_VERSION
    QApplication::setApplicationVersion(QString("%1").arg(STR_VERSION));
#endif
    QApplication::setOrganizationDomain("lxqt");
    QSettings::setDefaultFormat(QSettings::NativeFormat);

    QApplication app(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        QMessageBox::critical(0, QObject::tr("Systray"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }

    QApplication::setQuitOnLastWindowClosed(false);

    NetworkManagerApplet applet;
    applet.show();

    return app.exec();
}

