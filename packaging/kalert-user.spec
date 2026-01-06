%global debug_package %{nil}

Name:           kalert-user
Version:        1.0.0
Release:        1%{?dist}
Summary:        Kalert user space library and daemon program

License:        LGPL-2.1-or-later
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  pkgconfig
BuildRequires:  libmnl-devel
BuildRequires:  libev-devel

Provides:       libkalert.so

%description
kalert-user provides user space support for Kalert kernel features:
- libkalert library for application integration
- kalertd daemon to monitor and handle kernel events

%prep
%autosetup -n %{name}-%{version}

%build
%make_build

%install
%make_install DESTDIR=%{buildroot} PREFIX=%{_prefix} libdir=%{_libdir}

# install pkgconfig file
mkdir -p %{buildroot}%{_libdir}/pkgconfig
sed -e "s|@prefix@|%{_prefix}|" \
    -e "s|@libdir@|%{_libdir}|" \
    packaging/libkalert.pc.in > %{buildroot}%{_libdir}/pkgconfig/libkalert.pc

# install systemd unit
mkdir -p %{buildroot}%{_unitdir}
install -m 0644 packaging/kalertd.service %{buildroot}%{_unitdir}/kalertd.service

# install configuration files
mkdir -p %{buildroot}%{_sysconfdir}/kalert
cp -a packaging/conf/* %{buildroot}%{_sysconfdir}/kalert/

%post
/sbin/ldconfig
%systemd_post kalertd.service

%preun
%systemd_preun kalertd.service

%postun
/sbin/ldconfig
%systemd_postun_with_restart kalertd.service

%files
%license COPYING
%doc README.md
%{_bindir}/kalertd
%{_bindir}/sub_test
%{_libdir}/libkalert.so
%{_libdir}/libkalert.so.*
%{_libdir}/libkalert.a
%{_includedir}/libkalert/
%{_libdir}/pkgconfig/libkalert.pc
%{_unitdir}/kalertd.service
%config(noreplace) %{_sysconfdir}/kalert/*

%changelog
* Mon Aug 18 2025 Huiwen He <hehuiwen@kylinos.cn> - 1.0.0-1
- Initial RPM release

