create table data_collect (
    `Id` bigint(20) NOT NULL AUTO_INCREMENT primary key COMMENT '编号',
    ip varchar(20) not null default '',
    time datetime not null default '2000-00-00 00:00:00',
    cpu varchar(200) not null default '',
    mem varchar(200) not null default '',
    swap varchar(200) not null default '',
    partion varchar(200) not null default '',
    tcp varchar(200) not null default '',
    udp varchar(200) not null default '',
    traffic varchar(200) not null default '',
    loadavg varchar(200) not null default '',
    io varchar(200) not null default ''
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
