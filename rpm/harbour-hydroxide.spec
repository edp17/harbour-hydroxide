Name:       harbour-hydroxide
Version:    1.0.0
Release:    0
Summary:    Local Proton Mail bridge (IMAP/SMTP) for Sailfish OS
License:    MIT
URL:        https://github.com/emersion/hydroxide
Group:      Applications/Internet

Requires:   systemd-user-session-targets

%description
Hydroxide provides a local IMAP/SMTP bridge for Proton Mail by communicating
with Proton's API. This package installs hydroxide and configures a
systemd user-session service for Sailfish OS.

The bridge listens only on localhost (127.0.0.1).

%prep
# nothing to unpack

%build
# binary package only

%install
rm -rf %{buildroot}

install -d %{buildroot}%{_bindir}
install -d %{buildroot}/usr/lib/systemd/user
install -d %{buildroot}/usr/lib/systemd/user/user-session.target.wants

%ifarch aarch64
install -m 0755 bin/hydroxide.aarch64 %{buildroot}%{_bindir}/hydroxide
%endif

%ifarch armv7hl armv7hnl armv7l
install -m 0755 bin/hydroxide.armv7hl %{buildroot}%{_bindir}/hydroxide
%endif

install -m 0644 %{_sourcedir}/../systemd/hydroxide.service \
    %{buildroot}/usr/lib/systemd/user/hydroxide.service

ln -s ../hydroxide.service \
    %{buildroot}/usr/lib/systemd/user/user-session.target.wants/hydroxide.service

%post
# Best effort: try to start the user service immediately.
# If no user manager is available yet, install must still succeed.
USER_NAME=""
if id defaultuser >/dev/null 2>&1; then
    USER_NAME="defaultuser"
elif id nemo >/dev/null 2>&1; then
    USER_NAME="nemo"
fi

if [ -n "$USER_NAME" ]; then
    su - "$USER_NAME" -c 'systemctl --user daemon-reload >/dev/null 2>&1 || :'
    su - "$USER_NAME" -c 'systemctl --user start hydroxide.service >/dev/null 2>&1 || :'
fi

%preun
# On final uninstall, stop the user service.
if [ "$1" -eq 0 ]; then
    USER_NAME=""
    if id defaultuser >/dev/null 2>&1; then
        USER_NAME="defaultuser"
    elif id nemo >/dev/null 2>&1; then
        USER_NAME="nemo"
    fi

    if [ -n "$USER_NAME" ]; then
        su - "$USER_NAME" -c 'systemctl --user stop hydroxide.service >/dev/null 2>&1 || :'
    fi
fi

%postun
# Keep user systemd state fresh after upgrade/remove.
USER_NAME=""
if id defaultuser >/dev/null 2>&1; then
    USER_NAME="defaultuser"
elif id nemo >/dev/null 2>&1; then
    USER_NAME="nemo"
fi

if [ -n "$USER_NAME" ]; then
    su - "$USER_NAME" -c 'systemctl --user daemon-reload >/dev/null 2>&1 || :'
fi

%files
%{_bindir}/hydroxide
/usr/lib/systemd/user/hydroxide.service
/usr/lib/systemd/user/user-session.target.wants/hydroxide.service

