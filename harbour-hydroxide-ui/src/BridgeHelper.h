#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QProcess>

class BridgeHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QString bridgePassword READ bridgePassword NOTIFY bridgePasswordChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QStringList users READ users NOTIFY usersChanged)
    Q_PROPERTY(QString lastActionReport READ lastActionReport NOTIFY lastActionReportChanged)

public:
    explicit BridgeHelper(QObject *parent = nullptr);

    QString statusText() const { return m_statusText; }
    QString lastError() const { return m_lastError; }
    QString bridgePassword() const { return m_bridgePassword; }
    bool busy() const { return m_busy; }

    Q_INVOKABLE void refreshStatus();
    Q_INVOKABLE void startService();
    Q_INVOKABLE void stopService();
    Q_INVOKABLE void restartService();

    Q_INVOKABLE void authAndGetBridgePassword(const QString &email, const QString &password);
    Q_INVOKABLE void copyToClipboard(const QString &text);

    QStringList users() const { return m_users; }
    Q_INVOKABLE void refreshUsers();

    Q_INVOKABLE QStringList previewRemoveUsers(const QStringList &users);
    Q_INVOKABLE bool removeUsers(const QStringList &users);
    QString lastActionReport() const { return m_lastActionReport; }

    Q_INVOKABLE void authAndGetBridgePasswordPty(const QString &email, const QString &password);

signals:
    void statusChanged();
    void lastErrorChanged();
    void bridgePasswordChanged();
    void busyChanged();
    void usersChanged();
    void lastActionReportChanged();

private:
    void setBusy(bool b);
    void setError(const QString &e);

    // Runs: systemctl [--user] ... ; falls back to: systemctl --user -M nemo@ ...
    bool runSystemctl(const QStringList &args, QString *out, QString *err, int *exitCode);

    // Runs hydroxide auth and parses a password-like token from its output.
    QString extractBridgePassword(const QString &stdoutText) const;

    QString m_statusText;
    QString m_lastError;
    QString m_bridgePassword;
    bool m_busy{false};
    QStringList m_users;
    QStringList configFilesForUser(const QString &user) const;
    QString m_lastActionReport;
    QProcess *m_authProc = nullptr;
    QString m_authOutput;
};
