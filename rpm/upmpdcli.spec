Summary:        UPnP Media Renderer front-end to MPD, the Music Player Daemon
Name:           upmpdcli
Version:        0.4
Release:        1%{?dist}
Group:          Applications/Multimedia
License:        GPLv2+
URL:            http://www.lesbonscomptes.com/updmpdcli
Source0:        http://www.lesbonscomptes.com/upmpdcli/downloads/upmpdcli-%{version}.tar.gz
BuildRequires:  libupnp-devel
BuildRequires:  libmpdclient-devel
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%description
Upmpdcli turns MPD, the Music Player Daemon into an UPnP Media Renderer,
usable with most UPnP Control Point applications, such as those which run
on Android tablets or phones.

%prep
%setup -q

%build
%configure
%{__make} %{?_smp_mflags}
%{__rm} -f %{buildroot}/usr/lib64/libupnpp.a
%{__rm} -f %{buildroot}/usr/lib64/libupnpp.la

%install
%{__rm} -rf %{buildroot}
%{__make} install DESTDIR=%{buildroot} STRIP=/bin/true INSTALL='install -p'
%{__rm} -f %{buildroot}/usr/lib64/libupnpp.a
%{__rm} -f %{buildroot}/usr/lib64/libupnpp.la
%{__rm} -f %{buildroot}/etc/upmpdcli.conf

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-, root, root, -)
%{_bindir}/%{name}
%{_libdir}/libupnpp.so*
%{_datadir}/%{name}
%{_datadir}/%{name}/*
%{_mandir}/man1/%{name}.1*

%changelog
* Wed Feb 12 2014 J.F. Dockes <jf@dockes.org> - 0.4
- Version 0.4
