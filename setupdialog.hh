#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include "settings.hh"

namespace Ui {
class SetupDialog;
}

class SetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetupDialog(QWidget *parent = 0);
    ~SetupDialog();

public slots:
    void showSettings();

signals:
    void settings_changed(const SettingsPtr&);

private:
    Ui::SetupDialog *ui;
    void loadSettings();
    QSharedPointer<Settings> settings;

    void getIconFileName(QLineEdit* le, QLabel* im);
    void getExternalFileName(QLineEdit* le);

private slots:
    virtual void accept();
    void btIconClicked();
    void btExternalClicked();
};

#endif // SETUPDIALOG_H
