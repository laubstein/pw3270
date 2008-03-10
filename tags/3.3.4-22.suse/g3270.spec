Name:           g3270
License:        GPL
Group:          System/X11/Terminals
Version:        3.3.4
Release:        22.%_vendor
Summary:        IBM 3270 Terminal emulator for gtk.
Source:         %{name}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

Requires:       openssl %{name}-lib
BuildRequires:  openssl-devel sed pkgconfig ncurses-devel

%description
IBM 3270 terminal emulator gtk. It can be used to communicate with
any IBM host that supports 3270-style connections over TELNET.

%package lib
Provides:	%{name}-lib
Summary:	3270 Communication library for %{name}
Group:		Development/Libraries

%description lib
tn3270 protocol library for %{name}

%package devel
Provides:	%{name}-devel
Summary:	Devel for 3270 Communication library for %{name}
Group:		Development/Libraries

%description devel
devel for tn3270 protocol library for %{name}

%prep

%setup -q -n %{name}
./configure
cd src/lib
%configure --disable-trace
cd ../..

%build
make Release
strip bin/Release/g3270
strip bin/lib3270.so

%install
rm -rf $RPM_BUILD_ROOT

mkdir -p %{buildroot}%{_libdir}
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_datadir}/%{name}
mkdir -p %{buildroot}%{_datadir}/%{name}/plugins
mkdir -p %{buildroot}%{_includedir}/%{name}
mkdir -p %{buildroot}%{_sysconfdir}/x3270
mkdir -p %{buildroot}/etc/sysconfig
mkdir -p %{buildroot}%{_libdir}/pkgconfig

install -m 755 bin/lib3270.so	   	%{buildroot}%{_libdir}
install -m 755 bin/Release/g3270    %{buildroot}%{_datadir}/%{name}
install -m 644 src/*.jpg		    %{buildroot}%{_datadir}/%{name}
install -m 644 *.conf		    	%{buildroot}%{_datadir}/%{name}
install -m 644 src/lib/ibm_hosts	%{buildroot}%{_sysconfdir}/x3270
install -m 644 sysconfig		    %{buildroot}/etc/sysconfig/%{name}

install -m 644 src/lib/*.h			%{buildroot}%{_includedir}/%{name}
install -m 644 src/extension.h		%{buildroot}%{_includedir}/%{name}

sed "s@./src@%{_datadir}/%{name}@g" g3270.sh > %{buildroot}%{_datadir}/%{name}/%{name}.sh
chmod 755 %{buildroot}%{_datadir}/%{name}/%{name}.sh

ln -s %{_datadir}/%{name}/%{name}.sh %{buildroot}%{_bindir}/%name

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

# PKG-Config file
cat > %{name}.pc << EOF
prefix=%{_prefix}
exec_prefix=%{_exec_prefix}
libdir=%{_libdir}
includedir=%{_includedir}/%{name}
target=x11
extension_prefix=%{_datadir}/%{name}/plugins
extension_data=%{_datadir}/%{name}
startup_script=%{_datadir}/%{name}/%{name}.sh

Name: %{name}
Description: GTK-3270 Terminal Emulator
Version: %{version}-%{release}
Requires: gtk+-2.0
Libs: -L\${libdir} -l3270
Cflags: -I\${includedir}/ -DG3270=\"%{version}-%{release}\" -DPLUGINPATH=\"%{_datadir}/%{name}/plugins\" -DPLUGINDATA=\"%{_datadir}/%{name}\"
EOF

install -m 644 %{name}.pc %{buildroot}%{_libdir}/pkgconfig/%{name}.pc

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}
%{_datadir}/applications

%{_datadir}/%{name}/%{name}
%{_datadir}/%{name}/*.jpg
%{_datadir}/%{name}/*.sh

%config(noreplace) %{_datadir}/%{name}/*.conf
%config(noreplace) /etc/sysconfig/%{name}

%files lib
%defattr(-,root,root)
%{_libdir}
%{_sysconfdir}/x3270

%files devel
%{_includedir}/%{name}
%{_libdir}/pkgconfig


