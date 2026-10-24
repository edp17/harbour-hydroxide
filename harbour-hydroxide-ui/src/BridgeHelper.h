#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QProcess>

/**
 * @brief BridgeHelper manages the local Hydroxide service and handles authentication
 *        with ProtonMail, including human verification (CAPTCHA) via Sailfish OS WebView
 *        using Jahroots' hydroxide fork (https://github.com/Jahroots/hydroxide).
 */
class BridgeHelper : public QObject
{
    Q_OBJECT

    // Service & Process state
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QString bridgePassword READ bridgePassword NOTIFY bridgePasswordChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QStringList users READ users NOTIFY usersChanged)
    Q_PROPERTY(QString lastActionReport READ lastActionReport NOTIFY lastActionReportChanged)

    // CAPTCHA & Human Verification properties (Jahroots hydroxide fork support)
    Q_PROPERTY(bool captchaRequired READ captchaRequired NOTIFY captchaRequiredChanged)
    Q_PROPERTY(QString captchaUrl READ captchaUrl NOTIFY captchaUrlChanged)
    Q_PROPERTY(QString captchaStatus READ captchaStatus NOTIFY captchaStatusChanged)

public:
    explicit BridgeHelper(QObject *parent = nullptr);
    virtual ~BridgeHelper();

    // Property getters
    QString statusText() const { return m_statusText; }
    QString lastError() const { return m_lastError; }
    QString bridgePassword() const { return m_bridgePassword; }
    bool busy() const { return m_busy; }
    QStringList users() const { return m_users; }
    QString lastActionReport() const { return m_lastActionReport; }

    bool captchaRequired() const { return m_captchaRequired; }
    QString captchaUrl() const { return m_captchaUrl; }
    QString captchaStatus() const { return m_captchaStatus; }

    // Service management invokables
    Q_INVOKABLE void refreshStatus();
    Q_INVOKABLE void startService();
    Q_INVOKABLE void stopService();
    Q_INVOKABLE void restartService();

    // Authentication invokables
    Q_INVOKABLE void authAndGetBridgePassword(const QString &email, const QString &password);
    Q_INVOKABLE void authAndGetBridgePasswordPty(const QString &email, const QString &password);
    Q_INVOKABLE void cancelAuth();

    // Human verification / CAPTCHA invokables
    Q_INVOKABLE void submitCaptchaToken(const QString &token);
    Q_INVOKABLE void onCaptchaCompleted();

    // Account & Utility invokables
    Q_INVOKABLE void copyToClipboard(const QString &text);
    Q_INVOKABLE void refreshUsers();
    Q_INVOKABLE QStringList previewRemoveUsers(const QStringList &users);
    Q_INVOKABLE bool removeUsers(const QStringList &users);

signals:
    void statusChanged();
    void lastErrorChanged();
    void bridgePasswordChanged();
    void busyChanged();
    void usersChanged();
    void lastActionReportChanged();

    // CAPTCHA signals
    void captchaRequiredChanged();
    void captchaUrlChanged();
    void captchaStatusChanged();
    void captchaRequested(const QString &url);

private:
    void setBusy(bool b);
    void setError(const QString &e);
    void resetCaptchaState();
    void processAuthStreamOutput(const QString &textChunk);

    // Runs: systemctl [--user] ... ; falls back to: systemctl --user -M nemo@ ...
    bool runSystemctl(const QStringList &args, QString *out, QString *err, int *exitCode);

    // Runs hydroxide auth and parses a password-like token from its output.
    QString extractBridgePassword(const QString &stdoutText) const;
    QStringList configFilesForUser(const QString &user) const;

    QString m_statusText;
    QString m_lastError;
    QString m_bridgePassword;
    bool m_busy{false};
    QStringList m_users;
    QString m_lastActionReport;

    // CAPTCHA state
    bool m_captchaRequired{false};
    QString m_captchaUrl;
    QString m_captchaStatus;

    // Async process management
    QProcess *m_authProc = nullptr;
    QString m_authOutput;
};
