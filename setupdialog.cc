#include <QFileDialog>
#include <QPixmap>
#include "setupdialog.hh"
#include "ui_setupdialog.h"

#include "settings.hh"


SetupDialog::SetupDialog(QWidget *parent) :  QDialog(parent),
                                             ui(new Ui::SetupDialog) {
    ui->setupUi(this);
}

SetupDialog::~SetupDialog() {
    delete ui;
}

void SetupDialog::showSettings()
{
    loadSettings();
    show();
}

void SetupDialog::loadSettings()
{
    settings = SettingsPtr(new Settings);
    ui -> leHost -> setText(settings -> host());
    ui -> sbPort -> setValue(settings -> port());
    ui -> leUser -> setText(settings -> user());
    ui -> lePw -> setText(settings -> pw());
    ui -> cbPreview -> setChecked(settings -> preview_enabled());
    ui -> sbPreviewTime -> setValue(settings -> preview_time());
    ui -> leExternalCmd -> setText(settings -> ext_app_cmd());
    ui -> leExternalApp -> setText(settings -> external_cmd());

    ui -> leMBox -> setText(settings -> mbox());
    ui -> sbTimeout -> setValue(settings -> timeout()/1000);
    ui -> sbReIdle -> setValue(settings -> re_idle_intervall()/60000);
    if (settings->use_recent())
        ui -> btUseRecent -> setChecked(true);
    else
        ui -> btNotUseRecent -> setChecked(true);
    ui -> cbDetectGMail -> setChecked(settings -> detect_gmail());
    ui -> cbUpdateAllways -> setChecked(settings -> update_always());
    ui -> cbAutoReconnect -> setChecked(settings -> auto_reconnect());

    QString s;
    s = settings -> icon_new();
    ui -> leNewmailIcon -> setText(s);
    ui -> imNewMailIcon -> setPixmap(s);
    s = settings -> icon_normal();
    ui -> leNormalIcon -> setText(s);
    ui -> imNormalIcon -> setPixmap(s);
    s = settings -> icon_error();
    ui -> leErrorIcon -> setText(s);
    ui -> imErrorIcon -> setPixmap(s);
    s = settings -> icon_disconnected();
    ui -> leDisconnectedIcon -> setText(s);
    ui -> imDisconnectedIcon -> setPixmap(s);

}

void SetupDialog::accept() {
    emit(settings_changed(settings));
    QDialog::accept();
}

void SetupDialog::getIconFileName(QLineEdit* le, QLabel* im) {
    QString fn = QFileDialog::getOpenFileName(this, tr("Open icon file") , le -> text(), tr("Image files (*.png *.jpg *.svg);;All files (*.*)"));
    if (!fn.isNull()) {
        le -> setText(fn);
        im -> setPixmap(QPixmap(fn));
    }
}

void SetupDialog::btIconClicked() {
    QObject* o = QObject::sender();
    if (o == ui-> btNewmailIcon)
        getIconFileName(ui -> leNewmailIcon, ui -> imNewMailIcon);
    else if (o == ui-> btNormalIcon)
        getIconFileName(ui -> leNormalIcon, ui -> imNormalIcon);
    else if (o == ui-> btErrorIcon)
        getIconFileName(ui -> leErrorIcon, ui -> imErrorIcon);
    else if (o == ui-> btDisconnectedIcon)
        getIconFileName(ui -> leDisconnectedIcon, ui -> imDisconnectedIcon);
}

void SetupDialog::getExternalFileName(QLineEdit* le) {
    QString fn = QFileDialog::getOpenFileName(this, tr("External program path"), le -> text().replace(QRegExp(" .*"),""));
    if (!fn.isNull()) {
        le -> setText(fn);
    }
}

void SetupDialog::btExternalClicked()
{
    QObject* o = QObject::sender();
    if (o == ui -> btExternalCmd)
        getExternalFileName(ui -> leExternalCmd);
    else if (o == ui -> btExternalApp)
        getExternalFileName(ui -> leExternalApp);
}
