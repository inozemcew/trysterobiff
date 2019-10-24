#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QStringList>
#include <QSharedPointer>

struct Options {
    bool debug;
    QString settings_path;

    Options() : debug(false) { }

    Options(const QStringList argv) : debug(false) {
        parse_args(argv);
    }

private:
    void parse_args(const QStringList argv);
    void help(const QString prog);
};

class Settings : public QObject
{
        Q_OBJECT
    private:
        bool reconnect_needed;

        QString m_host, m_user, m_pw, m_mbox;
        int m_port;
        int m_timeout;
        bool m_preview_enabled;
        int m_preview_time;
        int m_re_idle_intervall;
        bool m_use_recent;
        bool m_detect_gmail;
        bool m_update_always;
        bool m_auto_reconnect;
        QString m_root_cert, m_local_cert;
        QString m_external_cmd, m_ext_app_cmd;
        QString m_icon_new, m_icon_normal, m_icon_error, m_icon_disconnected;

        void loadSettings();
    public:
        explicit Settings(QObject *parent = 0);

        QString host()  { return m_host; }
        int port()      { return m_port; }
        QString user()  { return m_user; }
        QString pw()    { return m_pw; }
        QString mbox()  { return m_mbox; }
        int timeout()   { return m_timeout; }
        int re_idle_intervall() { return m_re_idle_intervall; }
        bool preview_enabled()  { return m_preview_enabled; }
        int preview_time()      { return m_preview_time; }
        bool use_recent()       { return m_use_recent; }
        bool detect_gmail()     { return m_detect_gmail; }
        bool update_always()    { return m_update_always; }
        bool auto_reconnect()   { return m_auto_reconnect; }
        QString local_cert()    { return m_local_cert; }
        QString root_cert()     { return m_root_cert; }
        QString ext_app_cmd()   { return m_ext_app_cmd; }
        QString external_cmd()  { return m_external_cmd; }
        QString icon_new()      { return m_icon_new; }
        QString icon_normal()   { return m_icon_normal; }
        QString icon_error()    { return m_icon_error; }
        QString icon_disconnected()     { return m_icon_disconnected; }


        static void setup_settings(const Options &opts);

        void set_host(const QString& value) { m_host = value; }
        void set_port(const int value) { m_port = value; }
        void set_user(const QString& value) { m_user = value; }
        void set_update_always(bool value) {
            m_update_always = value;
            reconnect_needed = true;
        }
        void set_preview_enabled(bool value) {
            m_preview_enabled = value;
            // save settings
        }
        void set_pw(const QString& value) { m_pw = value;}
        void set_mbox(const QString& value) { m_mbox = value;}

signals:

public slots:
};


typedef QSharedPointer<Settings> SettingsPtr;


#endif // SETTINGS_H
