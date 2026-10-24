#include "BridgeHelper.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QProcess>
#include <QRegularExpression>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSet>
#include <QTimer>
#include <QThread>
#include <QObject>
#include <QtConcurrent/QtConcurrent>
#include <QThreadPool>
#include <QRunnable>
#include <termios.h>
#include <sys/ioctl.h>
#include <pty.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

static const char *kServiceName = "hydroxide.service";
static const char *kAuthActionPrefix = "AUTHJSON:";

BridgeHelper::BridgeHelper(QObject *parent) : QObject(parent)
{
    refreshStatus();
    refreshUsers();
}

BridgeHelper::~BridgeHelper()
{
    if (m_authProc && m_authProc->state() != QProcess::NotRunning) {
        m_authProc->kill();
        m_authProc->waitForFinished(1000);
    }
}

void BridgeHelper::setBusy(bool b)
{
    if (m_busy == b) return;
    m_busy = b;
    emit busyChanged();
}

void BridgeHelper::setError(const QString &e)
{
    if (m_lastError == e) return;
    m_lastError = e;
    emit lastErrorChanged();
}

void BridgeHelper::resetCaptchaState()
{
    bool changed = false;
    if (m_captchaRequired) {
        m_captchaRequired = false;
        changed = true;
    }
    if (!m_captchaUrl.isEmpty()) {
        m_captchaUrl.clear();
        changed = true;
    }
    if (!m_captchaStatus.isEmpty()) {
        m_captchaStatus.clear();
        changed = true;
    }
    if (changed) {
        emit captchaRequiredChanged();
        emit captchaUrlChanged();
        emit captchaStatusChanged();
    }
}

/**
 * @brief processAuthStreamOutput analyzes real-time stdout/stderr from hydroxide.
 *
 * It immediately extracts:
 *  1. Any address beginning with 127.0.0.1 (e.g. "http://127.0.0.1:<port>/" or "127.0.0.1:<port>")
 *     and signals the UI to open the WebView, simultaneously stopping the spinning circle!
 *  2. Any replied "Bridge password: <password>" so the password is presented to the user
 *     without waiting for process exit.
 */
void BridgeHelper::processAuthStreamOutput(const QString &newChunk)
{
    m_authOutput += newChunk;

    // 1) Normalize text: strip ANSI escapes and normalize newlines
    QString cleanOutput = m_authOutput;
    cleanOutput.replace("\r\n", "\n");
    cleanOutput.replace('\r', '\n');
    cleanOutput.remove(QRegularExpression("\x1B\\[[0-9;]*[A-Za-z]"));

    // Immediately stop busy indicator as soon as human verification notice is detected
    if (cleanOutput.contains("requires human verification", Qt::CaseInsensitive) ||
        cleanOutput.contains("complete the captcha", Qt::CaseInsensitive)) {
        if (m_busy) {
            setBusy(false);
        }
    }

    // 2) Match any address beginning with 127.0.0.1 or http://127.0.0.1:
    // (Jahroots hydroxide human verification proxy server)
    static const QRegularExpression ipRegex(
        R"(((?:https?://)?(?:127\.0\.0\.1|localhost|\[::1\]):\d+(?:/[^\s\r\n]*)?))",
        QRegularExpression::CaseInsensitiveOption
    );

    QRegularExpressionMatch ipMatch = ipRegex.match(cleanOutput);
    if (ipMatch.hasMatch()) {
        QString detectedUrl = ipMatch.captured(1).trimmed();
        if (!detectedUrl.startsWith("http://", Qt::CaseInsensitive) && !detectedUrl.startsWith("https://", Qt::CaseInsensitive)) {
            detectedUrl.prepend("http://");
        }
        if (!detectedUrl.endsWith("/")) {
            detectedUrl.append("/");
        }

        // CRITICAL: Stop spinning circle immediately so the user can interact
        if (m_busy) {
            setBusy(false);
        }

        if (m_captchaUrl != detectedUrl) {
            m_captchaUrl = detectedUrl;
            m_captchaRequired = true;
            m_captchaStatus = QString("Address %1 detected. Opening in WebView...").arg(m_captchaUrl);

            emit captchaUrlChanged();
            emit captchaRequiredChanged();
            emit captchaStatusChanged();
            emit captchaRequested(m_captchaUrl);
        }
    }

    // 3) Check if Bridge password has already been output by hydroxide in real-time
    const QString extracted = extractBridgePassword(cleanOutput);
    if (!extracted.isEmpty()) {
        if (m_busy) {
            setBusy(false);
        }
        if (m_bridgePassword != extracted) {
            m_bridgePassword = extracted;
            resetCaptchaState();
            emit bridgePasswordChanged();
            refreshUsers();
            refreshStatus();
        }
    }
}

void BridgeHelper::authAndGetBridgePasswordPty(const QString &email, const QString &password)
{
    setError(QString());
    const QString user = email.trimmed();
    if (user.isEmpty()) {
        setError("Email is empty");
        setBusy(false);
        return;
    }
    if (password.isEmpty()) {
        setError("Password is empty");
        setBusy(false);
        return;
    }

    // Terminate any previous authentication process
    if (m_authProc && m_authProc->state() != QProcess::NotRunning) {
        m_authProc->kill();
        m_authProc->waitForFinished(1000);
    }

    resetCaptchaState();
    m_bridgePassword.clear();
    emit bridgePasswordChanged();
    setBusy(true);

    if (!m_authProc) {
        m_authProc = new QProcess(this);
        m_authProc->setProcessChannelMode(QProcess::MergedChannels);

        auto handleData = [this]() {
            if (!m_authProc) return;
            QByteArray data;
            data += m_authProc->readAllStandardOutput();
            data += m_authProc->readAllStandardError();
            data += m_authProc->readAll();
            if (!data.isEmpty()) {
                processAuthStreamOutput(QString::fromUtf8(data));
            }
        };

        connect(m_authProc, &QProcess::readyReadStandardOutput, this, handleData);
        connect(m_authProc, &QProcess::readyReadStandardError, this, handleData);
        connect(m_authProc, &QProcess::readyRead, this, handleData);

        connect(m_authProc, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
            Q_UNUSED(error);
            setBusy(false);
            if (m_authProc) {
                setError("Hydroxide process error: " + m_authProc->errorString());
            }
        });

        connect(m_authProc,
                static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                this,
                [this](int exitCode, QProcess::ExitStatus exitStatus) {
            if (m_authProc) {
                QByteArray remaining;
                remaining += m_authProc->readAllStandardOutput();
                remaining += m_authProc->readAllStandardError();
                remaining += m_authProc->readAll();
                if (!remaining.isEmpty()) {
                    m_authOutput += QString::fromUtf8(remaining);
                }
            }
            const QString out = m_authOutput;
            m_authOutput.clear();
            setBusy(false);
            resetCaptchaState();

            if (exitStatus != QProcess::NormalExit) {
                setError("hydroxide auth process terminated unexpectedly");
                return;
            }

            if (exitCode != 0) {
                if (out.contains("human verification wasn't accepted", Qt::CaseInsensitive)) {
                    setError("Proton human verification was not accepted or expired. Please try again.");
                } else if (out.contains("password", Qt::CaseInsensitive) && out.contains("invalid", Qt::CaseInsensitive)) {
                    setError("Invalid username or password for ProtonMail account.");
                } else {
                    setError("hydroxide auth failed.\n\nOutput:\n" + out.trimmed());
                }
                return;
            }

            const QString extracted = extractBridgePassword(out);
            if (!extracted.isEmpty()) {
                m_bridgePassword = extracted;
                emit bridgePasswordChanged();
                refreshUsers();
                refreshStatus();
            } else if (m_bridgePassword.isEmpty()) {
                setError("Could not extract bridge password from output.\n\nOutput:\n" + out.trimmed());
            }
        });
    }

    m_authOutput.clear();

    QString prog = "/usr/bin/hydroxide";
    if (!QFileInfo::exists(prog)) {
        prog = "hydroxide";
    }
    m_authProc->setProgram(prog);
    m_authProc->setArguments(QStringList() << "auth" << user);
    m_authProc->start();

    if (!m_authProc->waitForStarted(4000)) {
        setBusy(false);
        setError("Failed to start " + prog);
        return;
    }

    // Feed password to hydroxide via stdin pipe without closing write channel
    m_authProc->write(password.toUtf8());
    m_authProc->write("\n");
    m_authProc->waitForBytesWritten(2000);

    // Watchdog safety timer to prevent endless spinning circle if process hangs
    QTimer::singleShot(120000, this, [this]() {
        if (m_busy) {
            setBusy(false);
            if (m_authProc && m_authProc->state() != QProcess::NotRunning) {
                m_authProc->kill();
            }
            if (m_bridgePassword.isEmpty() && m_lastError.isEmpty()) {
                setError("Authentication timed out after 2 minutes.");
            }
        }
    });
}

void BridgeHelper::authAndGetBridgePassword(const QString &email, const QString &password)
{
    // Use the non-blocking implementation to keep UI responsive and allow WebView
    authAndGetBridgePasswordPty(email, password);
}

void BridgeHelper::cancelAuth()
{
    if (m_authProc && m_authProc->state() != QProcess::NotRunning) {
        m_authProc->kill();
        m_authProc->waitForFinished(1000);
    }
    resetCaptchaState();
    setBusy(false);
    setError("Authentication cancelled.");
}

void BridgeHelper::submitCaptchaToken(const QString &token)
{
    const QString clean = token.trimmed();
    if (clean.isEmpty()) return;

    if (m_authProc && m_authProc->state() == QProcess::Running) {
        m_authProc->write(clean.toUtf8());
        m_authProc->write("\n");
    }
}

void BridgeHelper::onCaptchaCompleted()
{
    m_captchaStatus = "CAPTCHA solved. Finalizing ProtonMail authentication...";
    emit captchaStatusChanged();
}

void BridgeHelper::copyToClipboard(const QString &text)
{
    QClipboard *cb = QGuiApplication::clipboard();
    if (cb) cb->setText(text);
}

QString BridgeHelper::extractBridgePassword(const QString &text) const
{
    // 1) Normalize line endings and strip ANSI escape sequences
    QString t = text;
    t.replace("\r\n", "\n");
    t.replace('\r', '\n');

    // Remove ANSI CSI sequences: ESC [ ... letter
    t.remove(QRegularExpression("\x1B\\[[0-9;]*[A-Za-z]"));

    // Remove other control chars except \n and \t
    t.remove(QRegularExpression("[\\x00-\\x08\\x0B\\x0C\\x0E-\\x1F\\x7F]"));

    // 2) Preferred: regex match "Bridge password: <token>"
    QRegularExpression re("(?i)\\bbridge\\s+password\\b\\s*:\\s*([^\\s]+)");
    QRegularExpressionMatch m = re.match(t);
    if (m.hasMatch()) {
        QString pw = m.captured(1).trimmed();

        // 3) Keep safe characters (base64 / alphanumeric)
        QString cleaned;
        cleaned.reserve(pw.size());
        for (int i = 0; i < pw.size(); ++i) {
            const QChar c = pw.at(i);
            if ((c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') ||
                c == '+' || c == '/' || c == '=' ||
                c == '-' || c == '_' ) {
                cleaned.append(c);
            }
        }
        return cleaned;
    }
    return QString();
}

static QString hydroxideConfigDir()
{
    return QDir::homePath() + "/.config/hydroxide";
}

static QString authJsonPath()
{
    return hydroxideConfigDir() + "/auth.json";
}

QStringList BridgeHelper::configFilesForUser(const QString &user) const
{
    const QString dirPath = hydroxideConfigDir();
    QDir dir(dirPath);
    if (!dir.exists())
        return {};

    const QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    QStringList matches;
    for (const QString &fn : files) {
        if (fn == user || fn.startsWith(user + ".")) {
            matches << dir.filePath(fn);
        }
    }
    return matches;
}

QStringList BridgeHelper::previewRemoveUsers(const QStringList &users)
{
    QStringList actions;
    const QString dirPath = hydroxideConfigDir();
    QDir dir(dirPath);
    const QStringList allFiles = dir.exists()
        ? dir.entryList(QDir::Files | QDir::NoDotAndDotDot)
        : QStringList{};

    for (const QString &u0 : users) {
        const QString u = u0.trimmed();
        if (u.isEmpty()) continue;

        if (QFileInfo::exists(authJsonPath()))
            actions << (QString(kAuthActionPrefix) + u);

        for (const QString &fn : allFiles) {
            if (fn.contains(u)) {
                actions << dir.filePath(fn);
            }
        }
    }
    actions.removeDuplicates();
    actions.sort();
    return actions;
}

bool BridgeHelper::removeUsers(const QStringList &users)
{
    setError(QString());
    m_lastActionReport.clear();

    auto localPart = [](const QString &email) -> QString {
        const int at = email.indexOf('@');
        return (at > 0) ? email.left(at) : email;
    };

    const QString configDir = hydroxideConfigDir();
    const QString authPath  = authJsonPath();

    QStringList toRemove;
    for (const QString &u0 : users) {
        const QString u = u0.trimmed();
        if (!u.isEmpty() && !toRemove.contains(u))
            toRemove << u;
    }

    if (toRemove.isEmpty()) {
        m_lastActionReport = "No users selected.";
        emit lastActionReportChanged();
        return false;
    }

    // Stop service first to avoid locked DB / races
    stopService();

    // 1) Remove entries from auth.json
    bool authChanged = false;
    QStringList removedFromAuth;
    QJsonObject authObj;

    if (!QFileInfo::exists(authPath)) {
        setError("auth.json not found: " + authPath);
    } else {
        QFile f(authPath);
        if (!f.open(QIODevice::ReadOnly)) {
            setError("Failed to open auth.json for reading.");
        } else {
            const QByteArray data = f.readAll();
            f.close();

            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (!doc.isObject()) {
                setError("auth.json is not a valid JSON object.");
            } else {
                authObj = doc.object();
                for (const QString &email : toRemove) {
                    if (authObj.contains(email)) {
                        authObj.remove(email);
                        removedFromAuth << email;
                        authChanged = true;
                    }
                }

                if (authChanged) {
                    QFile out(authPath);
                    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                        setError("Failed to open auth.json for writing.");
                        authChanged = false;
                    } else {
                        QJsonDocument newDoc(authObj);
                        out.write(newDoc.toJson(QJsonDocument::Indented));
                        out.close();
                    }
                }
            }
        }
    }

    // 2) Delete per-user .db files
    QSet<QString> remainingLocalparts;
    for (auto it = authObj.begin(); it != authObj.end(); ++it) {
        const QString remEmail = it.key();
        const QString lp = localPart(remEmail);
        if (!lp.isEmpty())
            remainingLocalparts.insert(lp);
    }

    int dbDeletedOk = 0;
    int dbDeletedFail = 0;
    QStringList dbFailed;

    if (!authObj.isEmpty()) {
        for (const QString &email : toRemove) {
            const QString lp = localPart(email);
            if (lp.isEmpty() || remainingLocalparts.contains(lp))
                continue;

            const QString dbPath = configDir + "/" + lp + ".db";
            if (!QFileInfo::exists(dbPath))
                continue;

            if (QFile::remove(dbPath)) {
                dbDeletedOk++;
            } else {
                dbDeletedFail++;
                dbFailed << dbPath;
            }
        }
    }

    // Restart service after changes
    startService();
    refreshUsers();
    refreshStatus();

    // Report
    QString report;
    if (authChanged) {
        report += "Removed from auth.json:\n- " + removedFromAuth.join("\n- ") + "\n";
    } else {
        report += "No matching entries removed from auth.json.\n";
    }
    report += QString("Deleted orphan DB files: %1 ok, %2 failed.").arg(dbDeletedOk).arg(dbDeletedFail);
    m_lastActionReport = report;
    emit lastActionReportChanged();

    if (dbDeletedFail > 0) {
        setError("Failed to delete DB file(s):\n" + dbFailed.join("\n"));
    }

    return authChanged || (dbDeletedOk > 0);
}

void BridgeHelper::refreshUsers()
{
    setError(QString());
    QProcess p;
    p.setProgram("/usr/bin/hydroxide");
    p.setArguments(QStringList() << "status");
    p.start();

    if (!p.waitForFinished(8000)) {
        p.kill();
        p.waitForFinished(1000);
        setError("hydroxide status timed out");
        return;
    }

    const QString out = QString::fromUtf8(p.readAllStandardOutput());
    const QString err = QString::fromUtf8(p.readAllStandardError());
    if (!err.trimmed().isEmpty()) {
        setError(err.trimmed());
    }

    QStringList users;
    const QStringList lines = out.split('\n');
    for (const QString &line : lines) {
        QString s = line.trimmed();
        if (s.startsWith("- ")) {
            s = s.mid(2).trimmed();
            if (!s.isEmpty())
                users.append(s);
        }
    }
    users.sort(Qt::CaseInsensitive);

    if (m_users != users) {
        m_users = users;
        emit usersChanged();
    }
}

bool BridgeHelper::runSystemctl(const QStringList &args, QString *out, QString *err, int *exitCode)
{
    auto run = [&](const QStringList &fullArgs, QString *o, QString *e, int *c) -> bool {
        QProcess p;
        p.setProgram("systemctl");
        p.setArguments(fullArgs);
        p.start();
        if (!p.waitForFinished(8000)) {
            p.kill();
            p.waitForFinished(1000);
            if (e) *e = "systemctl timeout";
            if (c) *c = -1;
            return false;
        }
        if (o) *o = QString::fromUtf8(p.readAllStandardOutput());
        if (e) *e = QString::fromUtf8(p.readAllStandardError());
        if (c) *c = p.exitCode();
        return (p.exitStatus() == QProcess::NormalExit);
    };

    QString o1, e1; int c1 = 0;
    bool ok1 = run(QStringList() << "--user" << args, &o1, &e1, &c1);
    if (ok1) {
        if (out) *out = o1;
        if (err) *err = e1;
        if (exitCode) *exitCode = c1;
        return true;
    }
    return run(QStringList() << "--user" << "-M" << "nemo@" << args, out, err, exitCode);
}

void BridgeHelper::refreshStatus()
{
    setError(QString());
    QString out, err; int code = 0;
    bool ran = runSystemctl(QStringList() << "is-active" << kServiceName, &out, &err, &code);
    QString st = out.trimmed();
    if (!ran) {
        m_statusText = "Unknown";
        emit statusChanged();
        setError(err.isEmpty() ? "systemctl failed" : err.trimmed());
        return;
    }
    if (st.isEmpty()) st = "unknown";
    m_statusText = st;
    emit statusChanged();
    if (!err.trimmed().isEmpty())
        setError(err.trimmed());
}

void BridgeHelper::startService()
{
    setError(QString());
    QString out, err; int code = 0;
    runSystemctl(QStringList() << "start" << kServiceName, &out, &err, &code);
    if (!err.trimmed().isEmpty()) setError(err.trimmed());
    refreshStatus();
}

void BridgeHelper::stopService()
{
    setError(QString());
    QString out, err; int code = 0;
    runSystemctl(QStringList() << "stop" << kServiceName, &out, &err, &code);
    if (!err.trimmed().isEmpty()) setError(err.trimmed());
    refreshStatus();
}

void BridgeHelper::restartService()
{
    setError(QString());
    QString out, err; int code = 0;
    runSystemctl(QStringList() << "restart" << kServiceName, &out, &err, &code);
    if (!err.trimmed().isEmpty()) setError(err.trimmed());
    refreshStatus();
}
