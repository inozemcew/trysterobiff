/* {{{

    This file is part of trysterobiff -
      a cross-plattform non-polling IMAP new-mail systray notifier.

    Copyright (C) 2011  Georg Sauthoff
         email: mail@georg.so or gsauthof@sdf.lonestar.org

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

}}} */



#include <QApplication>
#include <QSystemTrayIcon>
#include <QSslSocket>
#include <QTimer>
#include <QSettings>
#include <QTextCodec>

#include "tray.hh"
#include "client.hh"
#include "external.hh"

#include "dummy.hh"
#include "name.hh"

#include <iostream>

using namespace std;


int main(int argc, char **argv) {

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(IMAPBIFFNAME);
    QCoreApplication::setApplicationName(IMAPBIFFNAME);
    app.setQuitOnLastWindowClosed(false);

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        std::cerr << "Could not find a System Tray.\n";
        return 42;
    }
    if (!QSslSocket::supportsSsl()) {
        std::cerr << "WTF?!? Qt lib does not support SSL ...\n";
        return 23;
    }

    Options opts(QApplication::arguments());
    Settings::setup_settings(opts);

    SettingsPtr s(new Settings);

    Tray t(s);
    Client c(s);
    External e(s);
    SetupDialog sd;

#ifndef NOIMAPDEBUG
    QObject::connect(&c, SIGNAL(debug(const QString&)), &t, SLOT(debug(const QString&)));
    if (opts.debug) {
        Dummy *d = new Dummy();
        QObject::connect(&c, SIGNAL(debug(const QString&)), d, SLOT(error(const QString&)));
        QObject::connect(&c, SIGNAL(error(const QString&)), d, SLOT(error(const QString&)));
        QObject::connect(&c, SIGNAL(new_messages(size_t)),  d, SLOT(new_messages(size_t)));
    }
#endif

    QObject::connect(&c, SIGNAL(error(const QString&)), &t, SLOT(error(const QString&)));
    QObject::connect(&c, SIGNAL(new_messages(size_t)), &t, SLOT(new_messages(size_t)));
    QObject::connect(&c, SIGNAL(new_headers(const QByteArray&)), &t, SLOT(new_headers(const QByteArray&)));
    QObject::connect(&c, SIGNAL(connected()), &t, SLOT(connected()));
    QObject::connect(&c, SIGNAL(disconnected()), &t, SLOT(disconnected()));

    QObject::connect(&t, SIGNAL(preview_toggled(bool)), &c, SLOT(preview_toggle(bool)));
    QObject::connect(&t, SIGNAL(connect_requested()), &c, SLOT(do_connect()));
    QObject::connect(&t, SIGNAL(disconnect_requested()), &c, SLOT(do_disconnect()));

    QObject::connect(&c, SIGNAL(new_messages(size_t)), &e, SLOT(new_messages(size_t)));
    QObject::connect(&t, SIGNAL(run_ext_app(const QString&)), &e, SLOT(ext_app(const QString&)));

    QObject::connect(&t, SIGNAL(show_settings()), &sd, SLOT(showSettings()));

    QObject::connect(&sd, SIGNAL(settings_changed(const SettingsPtr&)), &t, SLOT(new_settings(const SettingsPtr&)));

    c.start();
    return app.exec();
}

