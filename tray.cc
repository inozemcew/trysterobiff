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

Tray::Tray() : tray(0), con(0), discon(0), action_info(0), infobox(0),
               new_msg(0), show_preview(true), preview_time(5),
               pre_reconnect(false) {
    tray = new QSystemTrayIcon(); // WTF?!? crashes in QObjectPrivate::deleteChildren when: this);

    setup_infobox();

    QSettings s;

    QVariant v = s.value("external_app");
    if (v.isValid()) {
        ext_app_cmd = v.toString();
    }

    setup_menu();

    pixmap_new = QPixmap(s.value("gui/newmail", QVariant(":/icons/mail-message-new.svg")).toString());
    icon_normal = QIcon(s.value("gui/normal", QVariant(":/icons/mail-forward.svg")).toString());
    icon_error = QIcon(s.value("gui/error", QVariant(":/icons/process-stop.svg")).toString());
    icon_disconnected = QIcon(s.value("gui/disconnected", QVariant(":/icons/applications-multimedia.svg")).toString());

    show_preview = s.value("preview", QVariant(true)).toBool();
    preview_time = s.value("preview_time", QVariant(5)).toUInt();
    if (!preview_time)
        preview_time = 5;



    tray->setIcon(icon_normal);
    connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(action(QSystemTrayIcon::ActivationReason)));
    tray->show();
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
    QAction *ext_cmd = new QAction(tr("Run ") + ext_app_cmd, this);
    connect (ext_cmd, SIGNAL(triggered()), this, SLOT(run_ext_app()));

    QAction *info = new QAction(tr("&Diagnostics ..."), this);
    info->setCheckable(true);
    info->setChecked(false);
    // triggered is only emitted on user actions
    // connect(info, SIGNAL(toggled(bool)), infobox, SLOT(setVisible(bool)));
    connect(info, SIGNAL(triggered(bool)), infobox, SLOT(setVisible(bool)));
    action_info = info;
    connect(infobox, SIGNAL(hidden()), action_info, SLOT(toggle()));

    con = new QAction(tr("&Connect"), this);
    con->setEnabled(false);
    connect(con, SIGNAL(triggered()), this, SIGNAL(connect_requested()));

    discon = new QAction(tr("&Disconnect"), this);
    discon -> setEnabled(false);
    connect(discon, SIGNAL(triggered()), this, SIGNAL(disconnect_requested()));

    QAction *preview = new QAction(tr("&Enable preview"), this);
    preview->setCheckable(true);
    preview->setChecked(show_preview);
    connect(preview, SIGNAL(toggled(bool)), this, SIGNAL(preview_toggled(bool)));
    connect(preview, SIGNAL(toggled(bool)), this, SLOT(preview_toggle(bool)));

    QAction *ab = new QAction(tr("&About ..."), this);
    connect(ab, SIGNAL(triggered()), this, SLOT(about()));

    QAction *quit = new QAction(tr("&Quit"), this);
    connect(quit, SIGNAL(triggered()), qApp, SLOT(quit()));

    QSettings s;
    QWidgetAction *host = new QWidgetAction(this);
    QLabel *label = new QLabel(s.value("host").toString());
    host->setDefaultWidget(label);

    QAction *settings = new QAction(tr("&Settings ..."), this);
    connect(settings, SIGNAL(triggered()), this, SIGNAL(show_settings()));

    QMenu *menu = new QMenu(dynamic_cast<QWidget*>(tray));
    menu->addAction(host);
    menu->addSeparator();
    menu->addAction(ext_cmd);
    menu->addAction(info);
    menu->addAction(con);
    menu->addAction(discon);
    menu->addAction(preview);
    menu->addAction(settings);
    menu->addSeparator();
    menu->addAction(ab);
    menu->addSeparator();
    menu->addAction(quit);

    tray->setContextMenu(menu);
}

Tray::~Tray() {
    delete tray;
}

void Tray::run_ext_app() {
    emit run_ext_app(ext_app_cmd);
}

void Tray::reconnect() {
    pre_reconnect = true;
    emit disconnect_requested();
}

void Tray::action(QSystemTrayIcon::ActivationReason r) {
    std::cerr << "Clicked "<< r;
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

void Tray::show_message() {
    if (!new_msg || !show_preview)
        return;
    tray->showMessage(QString("New messages"), Qt::escape(QString::fromUtf8(headers)),
                      QSystemTrayIcon::Information, preview_time * 1000);
}

void Tray::connected() {
    tray->setIcon(icon_normal);
    if (tray->toolTip() == "Disconnected")
        tray->setToolTip("Connected");
    con->setEnabled(false);
    discon->setEnabled(true);
}


void Tray::disconnected() {
    tray->setIcon(icon_disconnected);
    tray->setToolTip("Disconnected");
    headers.clear();
    new_msg = 0;
    con->setEnabled(true);
    discon->setEnabled(false);
    if (pre_reconnect) {
        pre_reconnect = false;
        emit connect_requested();
    }
}

void Tray::preview_toggle(bool b) {
    show_preview = b;
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


