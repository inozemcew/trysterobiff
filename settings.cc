
#include <QDir>
#include "iostream"

#include "settings.hh"
#include "name.hh"

using namespace std;

Settings::Settings(QObject *parent) : QObject(parent), reconnect_needed(false),
                                      m_port(0), m_timeout(30 * 1000),
                                      m_preview_enabled(true), m_re_idle_intervall(28 * 60 * 1000),
                                      m_use_recent(true),
                                      m_detect_gmail(true),
                                      m_update_always(false),
                                      m_auto_reconnect(false) {
    loadSettings();
}

void Settings::loadSettings() {
    QSettings s;
    m_host = s.value("host").toString();
    m_port = s.value("port").toInt();
    m_user = s.value("user").toString();
    m_pw = s.value("pw").toString();
    m_mbox = s.value("mbox", "INBOX").toString();
    m_preview_enabled = s.value("preview", true).toBool();
    m_preview_time = s.value("preview_time",5).toInt();
    m_timeout = s.value("timeout", 30).toInt()*1000;
    m_re_idle_intervall = s.value("re_idle", 28).toInt()*60*1000;
    m_use_recent = s.value("use_recent", true).toBool();
    m_detect_gmail = s.value("detect_gmail", true).toBool();
    m_update_always = s.value("update_always", false).toBool();
    m_auto_reconnect = s.value("auto_reconnect", true).toBool();
    m_root_cert = s.value("cert").toString();
    m_local_cert = s.value("cert2").toString();
    m_external_cmd = s.value("external_cmd").toString();
    m_ext_app_cmd = s.value("external_app").toString();

    s.beginGroup("gui");
    m_icon_new = s.value("newmail", ":/icons/mail-message-new.svg").toString();
    m_icon_normal = s.value("normal", ":/icons/mail-forward.svg").toString();
    m_icon_error = s.value("error", ":/icons/process-stop.svg").toString();
    m_icon_disconnected = s.value("disconnected", ":/icons/applications-multimedia.svg").toString();
    s.endGroup();
}

void Settings::set_update_always(bool value) {
    m_update_always = value;
    reconnect_needed = true;
}

void Settings::set_preview_enabled(bool value) {
    m_preview_enabled = value;
    // save settings
}

void Settings::setup_settings(const Options& opts) {

    QSettings::setDefaultFormat(QSettings::IniFormat);
    if (!opts.settings_path.isEmpty()) {
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                           opts.settings_path);
    }

    QSettings s;
    if (!s.value("host").isValid()) {
        std::cerr << "Config file is missing. Copy example file to ";

        if (opts.settings_path.isEmpty())
            cerr << "$HOME/.config";
        else
            cerr << opts.settings_path.toUtf8().constData();

        cerr << "/" IMAPBIFFNAME ".ini,\n"
                "chmod 600 it and adjust the settings.\n";
        exit(6);
    }
/*
    QString cert = s.value("cert").toString();
    if (cert != "") {
        bool b = QSslSocket::addDefaultCaCertificates(cert);
        if (!b) {
            std::cerr << "Could not load additional root certificate: "
                      << cert.toUtf8().constData() << '\n';
            exit(7);
        }
    }*/
}

void Options::parse_args(const QStringList argv) {
    for (int i = 1; i<argv.size(); ++i)
        if (argv.at(i) == "--debug")
            debug = true;
        else if (argv.at(i) == "--help") {
            help(argv.at(0));
            exit(0);
        } else if (argv.at(i) == "--settings") {
            ++i;
            if (i>=argv.size()) {
                cerr << "Missing argument to --settings\n";
                exit(1);
            }
            settings_path = argv.at(i);
            QDir dir(settings_path);
            if (!dir.exists()) {
                cerr << "Settings directory '" << argv.at(i).data() << "' does not exist\n";
                exit(1);
            }
        } else {
            cerr << "Unknown option: " << argv.at(i).data() << '\n';
            help(argv.at(0));
            exit(1);
        }
}

void Options::help(const QString prog) {
    cerr << "Call: " << prog.data() <<
            " (Option)*\n\n"
            "where Option is one of:\n"
            "\n"
            //--------------------------------------------------------------------------------
            "  --help            this screen\n"
            "  --settings DIR    read trysterobiff.ini from DIR\n"
            "  --debug           print diagnostic output to stderr\n\n";
    //--------------------------------------------------------------------------------
}
