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

void BridgeHelper::authAndGetBridgePasswordPty(const QString &email, const QString &password)
{
    setError(QString());

    const QString user = email.trimmed();
    if (user.isEmpty()) {
        setError("Email is empty");
        return;
    }
    if (password.isEmpty()) {
        setError("Password is empty");
        return;
    }

    // If a previous auth is running, kill it.
    if (m_authProc && m_authProc->state() != QProcess::NotRunning) {
        m_authProc->kill();
        m_authProc->waitForFinished(1000);
    }

    if (!m_authProc) {
        m_authProc = new QProcess(this);

        // Merge stdout+stderr so we only parse one stream
        m_authProc->setProcessChannelMode(QProcess::MergedChannels);

        connect(m_authProc, &QProcess::readyRead, this, [this]() {
            m_authOutput += QString::fromUtf8(m_authProc->readAll());
        });

        connect(m_authProc,
                static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                this,
                [this](int exitCode, QProcess::ExitStatus exitStatus) {
            const QString out = m_authOutput;
            m_authOutput.clear();

            setBusy(false);

            if (exitStatus != QProcess::NormalExit) {
                setError("hydroxide auth crashed");
                return;
            }
            if (exitCode != 0) {
                setError("hydroxide auth failed.\n\nOutput:\n" + out.trimmed());
                return;
            }

            const QString extracted = extractBridgePassword(out);
            if (extracted.isEmpty()) {
                setError("Could not extract bridge password from output.\n\nOutput:\n" + out.trimmed());
                return;
            }

            m_bridgePassword = extracted;
            emit bridgePasswordChanged();

            refreshUsers();
            refreshStatus();
        });
    }

    // Start auth
    setBusy(true);
    m_bridgePassword.clear();
    emit bridgePasswordChanged();
    m_authOutput.clear();

    m_authProc->setProgram("/usr/bin/hydroxide");
    m_authProc->setArguments(QStringList() << "auth" << user);
    m_authProc->start();

    if (!m_authProc->waitForStarted(4000)) {
        setBusy(false);
        setError("Failed to start /usr/bin/hydroxide");
        return;
    }

    // With your patched hydroxide, stdin line will be accepted when no tty exists
    m_authProc->write(password.toUtf8());
    m_authProc->write("\n");
    m_authProc->closeWriteChannel();
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
    // Hydroxide config dir
    const QString dirPath = QDir::homePath() + "/.config/hydroxide";
    QDir dir(dirPath);
    if (!dir.exists())
        return {};

    // We will delete files that “clearly match” the user.
    // Rules:
    //  - exact basename match: <user>.<ext> (e.g. user@x.tld.db/json)
    //  - also any file whose name starts with the user + "." (same)
    //
    // We do NOT delete unrelated global files unless they are explicitly per-user.
    // (We avoid deleting generic auth caches blindly.)
    const QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    QStringList matches;

    for (const QString &fn : files) {
        if (fn == user) {
            matches << dir.filePath(fn);
            continue;
        }
        if (fn.startsWith(user + ".")) {
            matches << dir.filePath(fn);
            continue;
        }
    }

    return matches;
}

QStringList BridgeHelper::previewRemoveUsers(const QStringList &users)
{
    QStringList actions;

    // 1) Best-effort file deletion: any file whose name contains the user email
    //    (less strict than "startsWith user + '.'", works with more layouts)
    const QString dirPath = hydroxideConfigDir();
    QDir dir(dirPath);
    const QStringList allFiles = dir.exists()
        ? dir.entryList(QDir::Files | QDir::NoDotAndDotDot)
        : QStringList{};

    for (const QString &u0 : users) {
        const QString u = u0.trimmed();
        if (u.isEmpty()) continue;

        // add auth removal action if auth.json exists (we’ll check entry later)
        if (QFileInfo::exists(authJsonPath()))
            actions << (QString(kAuthActionPrefix) + u);

        for (const QString &fn : allFiles) {
            // "clearly match" — filename contains the email literally
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

    const QString configDir = QDir::homePath() + "/.config/hydroxide";
    const QString authPath  = configDir + "/auth.json";

    // Normalize requested removals
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

    // Stop service first to avoid locked db / races
    stopService();

    // --- 1) Remove entries from auth.json (if possible) ---
    bool authChanged = false;
    QStringList removedFromAuth;
    QJsonObject authObj;

    if (!QFileInfo::exists(authPath)) {
        // Not fatal — some layouts might differ, but removal can’t proceed safely.
        setError("auth.json not found: " + authPath);
    } else {
        QFile f(authPath);
        if (!f.open(QIODevice::ReadOnly)) {
            setError("Failed to open auth.json for reading.");
        } else {
            const QByteArray data = f.readAll();
            f.close();

            QJsonParseError pe{};
            QJsonDocument doc = QJsonDocument::fromJson(data, &pe);
            if (pe.error != QJsonParseError::NoError || !doc.isObject()) {
                setError("auth.json is not a JSON object (won't modify).");
            } else {
                authObj = doc.object();
                for (const QString &u : toRemove) {
                    if (authObj.contains(u)) {
                        authObj.remove(u);
                        authChanged = true;
                        removedFromAuth << u;
                    }
                }

                if (authChanged) {
                    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                        setError("Failed to open auth.json for writing.");
                    } else {
                        f.write(QJsonDocument(authObj).toJson(QJsonDocument::Indented));
                        f.close();
                    }
                }
            }
        }
    }

    // --- 2) Delete orphaned <localpart>.db files safely ---
    //
    // We only delete "<localpart>.db" if after auth.json update there is no remaining
    // account with the same localpart. This avoids the x@proton.me / x@protonmail.com
    // shared DB issue.

    // Build remaining localparts from updated authObj (only if we could parse it).
    QSet<QString> remainingLocalparts;
    if (!authObj.isEmpty()) {
        const QStringList remainingEmails = authObj.keys();
        for (const QString &email : remainingEmails) {
            remainingLocalparts.insert(localPart(email.trimmed()));
        }
    }

    int dbDeletedOk = 0;
    int dbDeletedFail = 0;
    QStringList dbFailed;

    // Only attempt db deletions if we have a parsed auth object (so we know "remaining")
    if (!authObj.isEmpty()) {
        for (const QString &email : toRemove) {
            const QString lp = localPart(email);
            if (lp.isEmpty())
                continue;

            // Still used by another remaining account -> do NOT delete db
            if (remainingLocalparts.contains(lp))
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

    // Refresh list shown in UI
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

    // Success if we changed auth or deleted at least one db
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
        // not fatal, but show it
        setError(err.trimmed());
    }

    // Example output:
    // "2 logged in user(s):"
    // "- a@proton.me"
    // "- b@protonmail.com"
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

    // systemctl outputs: active / inactive / failed / activating / deactivating
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

QString BridgeHelper::extractBridgePassword(const QString &text) const
{
    // 1) Normalize line endings and strip ANSI escape sequences (colors, cursor moves, etc.)
    QString t = text;
    t.replace("\r\n", "\n");
    t.replace('\r', '\n');

    // Remove ANSI CSI sequences: ESC [ ... letter
    // Example: "\x1b[0m", "\x1b[32m", etc.
    t.remove(QRegularExpression("\x1B\\[[0-9;]*[A-Za-z]"));

    // Remove other control chars except \n and \t
    t.remove(QRegularExpression("[\\x00-\\x08\\x0B\\x0C\\x0E-\\x1F\\x7F]"));

    // 2) Preferred: regex match "Bridge password: <token>"
    QRegularExpression re("(?i)\\bbridge\\s+password\\b\\s*:\\s*([^\\s]+)");
    QRegularExpressionMatch m = re.match(t);
    if (m.hasMatch()) {
        QString pw = m.captured(1).trimmed();

        // 3) Final safety: keep only characters that can appear in these tokens
        // (Base64-ish + urlsafe variants). This strips any remaining junk.
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

void BridgeHelper::authAndGetBridgePassword(const QString &email, const QString &password)
{
    setBusy(true);
    setError(QString());
    m_bridgePassword.clear();
    emit bridgePasswordChanged();

    if (email.trimmed().isEmpty()) {
        setError("Email is empty");
        setBusy(false);
        return;
    }
    if (password.isEmpty()) {
        setError("Password is empty");
        setBusy(false);
        return;
    }

    QProcess p;
    p.setProgram("/usr/bin/hydroxide");
    p.setArguments(QStringList() << "auth" << email.trimmed());
    p.start();

    if (!p.waitForStarted(4000)) {
        setError("Failed to start /usr/bin/hydroxide");
        setBusy(false);
        return;
    }

    // Feed password + newline
    p.write(password.toUtf8());
    p.write("\n");
    p.closeWriteChannel();

    if (!p.waitForFinished(60000)) {
        p.kill();
        p.waitForFinished(1000);
        setError("hydroxide auth timed out");
        setBusy(false);
        return;
    }

    const QString out = QString::fromUtf8(p.readAllStandardOutput());
    const QString err = QString::fromUtf8(p.readAllStandardError());

    const QString extracted = extractBridgePassword(out + "\n" + err);
    if (extracted.isEmpty()) {
        setError("Could not extract bridge password from output.\n\nOutput:\n" + (out + "\n" + err).trimmed());
        setBusy(false);
        return;
    }

    m_bridgePassword = extracted;
    emit bridgePasswordChanged();

    // Refresh status too (auth might create config etc.)
    refreshStatus();
    setBusy(false);
}

void BridgeHelper::copyToClipboard(const QString &text)
{
    QClipboard *cb = QGuiApplication::clipboard();
    if (cb) cb->setText(text);
}
