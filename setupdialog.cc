#include "setupdialog.hh"
#include "ui_setupdialog.h"

#include <QSettings>

SetupDialog::SetupDialog(QWidget *parent) :  QDialog(parent), ui(new Ui::SetupDialog) {
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
    QSettings settings;
    //settings.beginGroup("General");
    ui -> leHost -> setText(settings.value("host","<Enter your IMAP server address here>").toString());
    ui -> sbPort -> setValue(settings.value("port").toInt());
    ui -> leUser -> setText(settings.value("user").toString());
    ui -> lePw -> setText(settings.value("pw").toString());
    ui -> cbPreview -> setChecked(settings.value("preview", true).toBool());
    ui -> sbPreviewTime -> setValue(settings.value("preview_time",5).toInt());
    //settings.endGroup();
}
