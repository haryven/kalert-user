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

Provides:       libkalert.so

%description
kalert-user 提供 Kalert 内核特性的用户态支持，包括：
- libkalert 用户空间库，用于开发集成 kalert 功能的应用。
- kalertd 后台程序，负责监听和处理 kalert 内核事件。

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

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

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

%changelog
* Mon Aug 18 2025 Huiwen He <hehuiwen@kylinos.cn> - 1.0.0-1
- Initial RPM release

