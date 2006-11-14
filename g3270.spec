Name:           g3270
License:        GPL
Group:          System/Libraries
Version:        3.3
Release:        0.%_vendor
Summary:        IBM 3270 Terminal emulator for gtk.
Requires:       gtk+2 openssl
Source:         %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
BuildRequires:  gtk2-devel openssl-devel

%description
IBM 3270 terminal emulator gtk. It can be used to communicate with
any IBM host that supports 3270-style connections over TELNET. 

%package lib
Provides:	%{name}-lib
Summary:        3270 Communication library for %{name}
Group:          Development/Libraries

%description lib
tn3270 protocol library for %{name}

%prep

%setup -q -n %{name}
./configure
cd src/lib
./configure --disable-trace
cd ../..

%build
make DATADIR=/%{_datadir}/%{name} -C src

%install
rm -rf $RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT/%{_libdir}
mkdir -p $RPM_BUILD_ROOT/%{_bindir}
# mkdir -p $RPM_BUILD_ROOT/%{_datadir}/%{name}

install -m 755 src/lib3270.so	$RPM_BUILD_ROOT/%{_libdir}
install -m 755 src/%{name}	$RPM_BUILD_ROOT/%{_bindir}

%clean
rm -rf $RPM_BUILD_ROOT

%files lib
%{_libdir}

%files
%{_bindir}
# %{_datadir}/%{name}

