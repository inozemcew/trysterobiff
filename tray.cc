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


#include "tray.hh"

#include <QApplication>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QMessageBox>
#include <QWidgetAction>
#include <QLabel>
#include <QTextDocument>
#include <QPainter>
#include <QPainterPath>

#include <QDebug>

#include "name.hh"
#include "infobox.hh"
#include <iostream>

Tray::Tray(const SettingsPtr& aSettings ) : settings(0), tray(0), infobox(0),
               new_msg(0), pre_reconnect(false) {
    tray = new QSystemTrayIcon(); // WTF?!? crashes in QObjectPrivate::deleteChildren when: this);
    connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(action(QSystemTrayIcon::ActivationReason)));

    settings = aSettings;
    setup_infobox();
    setup_menu();
    setup_icons();

    tray->setIcon(icon_normal);
    tray->show();
}

void Tray::setup_icons() {
    pixmap_new = QPixmap(settings -> icon_new());
    icon_normal = QIcon(settings -> icon_normal());
    icon_error = QIcon(settings -> icon_error());
    icon_disconnected = QIcon(settings -> icon_disconnected());
}

QIcon Tray::icon_newmail(int count) {
    QPixmap pixmap(pixmap_new);
    QPainter painter(&pixmap);
    QPainterPath path;
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(Qt::white);

    QString countS(QString::number(count));
    QFont font("Arial",20);
    path.addText(QPoint(1,23), font, countS);
    painter.drawPath(path);

    painter.setPen(Qt::black);
    font.setPointSize(18);
    painter.setFont(font);
    painter.drawText(QPoint(2,23), countS);
    return QIcon(pixmap);
}

void Tray::setup_menu() {
    menuItems.ext_cmd = new QAction(tr("Run ") + (settings -> ext_app_cmd()), this);
    connect (menuItems.ext_cmd, SIGNAL(triggered()), this, SLOT(run_ext_app()));

    menuItems.info = new QAction(tr("&Diagnostics ..."), this);
    menuItems.info->setCheckable(true);
    menuItems.info->setChecked(false);
    // triggered is only emitted on user actions
    // connect(info, SIGNAL(toggled(bool)), infobox, SLOT(setVisible(bool)));
    connect(menuItems.info, SIGNAL(triggered(bool)), infobox, SLOT(setVisible(bool)));
    connect(infobox, SIGNAL(hidden()), menuItems.info, SLOT(toggle()));

    menuItems.con = new QAction(tr("&Connect"), this);
    menuItems.con->setEnabled(false);
    connect(menuItems.con, SIGNAL(triggered()), this, SIGNAL(connect_requested()));

    menuItems.discon = new QAction(tr("&Disconnect"), this);
    menuItems.discon -> setEnabled(false);
    connect(menuItems.discon, SIGNAL(triggered()), this, SIGNAL(disconnect_requested()));

    menuItems.preview = new QAction(tr("&Enable preview"), this);
    menuItems.preview->setCheckable(true);
    menuItems.preview->setChecked(settings -> preview_enabled());
    connect(menuItems.preview, SIGNAL(toggled(bool)), this, SIGNAL(preview_toggled(bool)));
    connect(menuItems.preview, SIGNAL(toggled(bool)), this, SLOT(preview_toggle(bool)));

    menuItems.about = new QAction(tr("&About ..."), this);
    connect(menuItems.about, SIGNAL(triggered()), this, SLOT(about()));

    menuItems.quit = new QAction(tr("&Quit"), this);
    connect(menuItems.quit, SIGNAL(triggered()), qApp, SLOT(quit()));

    menuItems.host = new QAction(settings -> host(), this);

    menuItems.settings = new QAction(tr("&Settings ..."), this);
    connect(menuItems.settings, SIGNAL(triggered()), this, SIGNAL(show_settings()));

    QMenu *menu = new QMenu();
    menu->addAction(menuItems.host);
    menu->setDefaultAction(menuItems.host);
    menu->addSeparator();
    menu->addAction(menuItems.ext_cmd);
    menu->addAction(menuItems.info);
    menu->addAction(menuItems.con);
    menu->addAction(menuItems.discon);
    menu->addAction(menuItems.preview);
    menu->addAction(menuItems.settings);
    menu->addSeparator();
    menu->addAction(menuItems.about);
    menu->addSeparator();
    menu->addAction(menuItems.quit);

    tray->setContextMenu(menu);
}

Tray::~Tray() {
    delete tray;
}

void Tray::run_ext_app() {
    emit run_ext_app(settings -> ext_app_cmd());
}

void Tray::reconnect() {
    pre_reconnect = true;
    emit disconnect_requested();
}

void Tray::action(QSystemTrayIcon::ActivationReason r) {
    switch (r) {
        case QSystemTrayIcon::Trigger :
            show_message();
            break;

        case QSystemTrayIcon::DoubleClick :
            emit run_ext_app();
            break;

        case QSystemTrayIcon::MiddleClick :
            //tray->setIcon(icon_normal);
            reconnect();
            break;

        default:
            break;
    }
}

void Tray::add_info(const QString &a) {
    infobox->add_line(a);
}

void Tray::error(const QString &a) {
    tray->setIcon(icon_error);
    add_info(a);
}


void Tray::debug(const QString &a) {
    add_info(a);
}

void Tray::new_messages(size_t i) {
    new_msg = i;
    tray->setToolTip(QString::number(i) + " new messages");
    if (i)
        tray->setIcon(icon_newmail(i));
    else {
        tray->setIcon(icon_normal);
        headers.clear();
    }
}

void Tray::new_headers(const QByteArray &a) {
    headers = a;
    show_message();
}

void Tray::new_settings(const SettingsPtr& aSettings)
{
    SettingsPtr old(settings);
    settings = aSettings;
    if (settings->host() != old ->host())
        menuItems.host->setText(settings->host());
    if (settings->ext_app_cmd() != old -> ext_app_cmd()) {
        menuItems.ext_cmd->setText(settings  -> ext_app_cmd());
    }
}

void Tray::show_message() {
    if (!new_msg || !settings -> preview_enabled())
        return;
    tray->showMessage(QString("New messages"), Qt::escape(QString::fromUtf8(headers)),
                      QSystemTrayIcon::Information, settings -> preview_time() * 1000);
}

void Tray::connected() {
    tray->setIcon(icon_normal);
    if (tray->toolTip() == "Disconnected")
        tray->setToolTip("Connected");
    menuItems.con->setEnabled(false);
    menuItems.discon->setEnabled(true);
}


void Tray::disconnected() {
    tray->setIcon(icon_disconnected);
    tray->setToolTip("Disconnected");
    headers.clear();
    new_msg = 0;
    menuItems.con->setEnabled(true);
    menuItems.discon->setEnabled(false);
    if (pre_reconnect) {
        pre_reconnect = false;
        emit connect_requested();
    }
}

void Tray::preview_toggle(bool b) {
    settings -> set_preview_enabled(b);
}

void Tray::setup_infobox() {
    infobox = new Infobox(dynamic_cast<QWidget*>(this));
}

void Tray::about() {
    QMessageBox::information(0, tr(IMAPBIFFNAME),
                             tr(IMAPBIFFNAME"\n"
                             "A non-polling IMAP new-mail systray notifier.\n"
                             "Copyright 2011,2013 Georg Sauthoff\n"
                             "Licenced under the GPLv3+."));
}


