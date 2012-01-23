Name:           g3270
License:        GPL
Group:          System/X11/Terminals
Version:        3.3.4
Release:        10.%_vendor
Summary:        IBM 3270 Terminal emulator for gtk.
Source:         %{name}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

Requires:       openssl %{name}-lib terminus-font
BuildRequires:  openssl-devel sed

%description
IBM 3270 terminal emulator gtk. It can be used to communicate with
any IBM host that supports 3270-style connections over TELNET.
$Revision$


%package lib
Provides:	%{name}-lib
Summary:        3270 Communication library for %{name}
Group:          Development/Libraries

%description lib
tn3270 protocol library for %{name}
$Revision$

%prep

%setup -q -n %{name}
./configure
cd src/lib
%configure --disable-trace
cd ../..

%build
make QUIET_MODE=1 DATADIR=%{_datadir}/%{name} VERSION=%{version}-%{release} TMPPATH=%{_tmppath} -C src
strip src/g3270
strip src/lib3270.so

%install
rm -rf $RPM_BUILD_ROOT

mkdir -p %{buildroot}%{_libdir}
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_datadir}/%{name}
mkdir -p %{buildroot}%{_sysconfdir}/x3270
mkdir -p %{buildroot}/etc/sysconfig

install -m 755 src/lib3270.so	    %{buildroot}%{_libdir}
install -m 755 src/g3270		    %{buildroot}%{_datadir}/%{name}/%{name}
install -m 644 src/*.jpg		    %{buildroot}%{_datadir}/%{name}
install -m 644 src/*.conf		    %{buildroot}%{_datadir}/%{name}
install -m 644 src/lib/ibm_hosts	%{buildroot}%{_sysconfdir}/x3270
install -m 644 sysconfig		    %{buildroot}/etc/sysconfig/%{name}

sed "s@./src@%{_datadir}/%{name}@g" g3270.sh > %{buildroot}%{_datadir}/%{name}/%{name}.sh
chmod 755 %{buildroot}%{_datadir}/%{name}/%{name}.sh

# Desktop menu entry
cat > %{name}.desktop << EOF
[Desktop Entry]
Encoding=UTF-8
Name=%{name}
Comment=IBM 3270 Terminal emulator
Exec=%{_datadir}/%{name}/%{name}.sh
Icon=%{_datadir}/%{name}/icon.jpg
Terminal=false
Type=Application
EOF

mkdir -p %{buildroot}%{_datadir}/applications

if [ %{_vendor} != conectiva ] ; then

	desktop-file-install    --vendor %{_build_vendor} \
                            --dir %{buildroot}%{_datadir}/applications \
                            --add-category System \
                            %{name}.desktop

else

	install -m 644 %{name}.desktop %{buildroot}%{_datadir}/applications

fi

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}
%{_datadir}/%{name}
%{_datadir}/applications
/etc/sysconfig/%{name}

%files lib
%defattr(-,root,root)
%{_libdir}
%{_sysconfdir}/x3270

%changelog

