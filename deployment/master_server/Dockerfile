# Use the official Ubuntu base image
FROM ubuntu:latest
 
# Set time zone (optional)
ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
 
RUN apt-get update --fix-missing && apt-get install -y apt-utils --fix-missing && apt-get install -y net-tools --fix-missing && apt-get install -y inetutils-ping --fix-missing &&
apt-get install -y telnet --fix-missing && apt-get install -y vim --fix-missing && apt-get install -y libssh2-1-dev --fix-missing

RUN apt-get install -y mysql-server --fix-missing
RUN echo "[client] \npoort   = 3310\n[mysqld]\nport    = 3310\n" >> /etc/mysql/my.cnf && /etc/init.d/mysql restart

RUN apt-get install -y apache2 --fix-missing && apt-get install -y php libapache2-mod-php --fix-missing && apt-get install -y php-mysql --fix-missing
RUN echo "ServerName 127.0.0.1:80" >> /etc/apache2/apache2.conf && /etc/init.d/apache2 restart

COPY QuixNet/mysql_script/centredb /root/workspace/QuixNet/mysql_script/centredb

RUN echo "#!/bin/bash\n\n/etc/init.d/mysql restart\ncd /root/workspace/QuixNet/mysql_script/centredb/\nmysql -h127.0.0.1 -p3310 -uroot < create_database.sql;\n
mysql -h127.0.0.1 -p3310 -uroot -Dnode_control_centre < node_control_centre.sql;\nmysql -h127.0.0.1 -p3310 -uroot < node_memcache.sql;" > /root/workspace/loadsql.sh && 
chmod a+x /root/workspace/loadsql.sh && ./root/workspace/loadsql.sh

COPY QuixNet/web/api/control /root/workspace/QuixNet/web/api/control
COPY QuixNet/web/index.html /root/workspace/QuixNet/web/index.html
COPY QuixNet/web/login_process.php /root/workspace/QuixNet/web/login_process.php
COPY QuixNet/web/register.html /root/workspace/QuixNet/web/register.html
COPY QuixNet/web/register.php /root/workspace/QuixNet/web/register.php

RUN cp -rf /root/workspace/QuixNet/web /var/www/html/QuixNet-control

RUN echo "<?php\n\
\t// mysql host\n\
\t\$ctrl_db_host = \"127.0.0.1\";\n\
\t// mysql user name\n\
\t\$ctrl_db_username = \"root\";\n\
\t// mysql password\n\
\t\$ctrl_db_password = \"\";\n\
\t// default database\n\
\t\$ctrl_db_database = \"node_control_contre\";\n\
\t// port\n\
\t\$ctrl_db_port = 3310;\n\
?>" > /var/www/html/QuixNet-control/db_config.php

# RUN CURL --INSECURE -sS https://getcomposer.org/download/latest-stable/composer.phar >> /root/composer.phar
COPY QuixNet/composer.phar /root/composer.phar


RUN echo "{\n\
\t\"repositories\":{\n\
\t\t\"packagist\":{\n\
\t\t\t\"type\":\"composer\",\n\
\t\t\t\"url\":\"http://mirrors.aliyun.com/composer/\"\n\
\t\t}\n\
\t},\n\
\t\t\"config\":{\n\
\t\t\t\"secure-http\":false\n\
\t\t}\n\
}\n" > /root/composer.json && mkdir -p /var/www/html/QuixNet-control/api/control && cp /root/composer.json /var/www/html/QuixNet-control/api/control/

RUN echo "#!/bin/bash\n\ncd /root/\nphp /root/composer.phar install\ncp /root/composer.phar /usr/local/bin/composer\nchmod a+x /usr/local/bin/composer
\nexport GIT_SSL_NO_VERIFY=1\ncd /var/www/html/QuixNet-control/api/control\ncp /root/composer.json composer.json\ncomposer require google/protobuf:3.11.4" > /root/
installPhpProtoBuf.sh && chmod a+x /root/installPhpProtoBuf.sh && ./root/installPhpProtoBuf.sh

COPY QuixNet/cmake_modules /root/workspace/QuixNet/cmake_modules
COPY QuixNet/shared /root/workspace/QuixNet/shared
COPY QuixNet/cache_server /root/workspace/QuixNet/cache_server
COPY QuixNet/servant_server /root/workspace/QuixNet/servant_server
COPY QuixNet/master_server /root/workspace/QuixNet/master_server
COPY QuixNet/ExitControlServers.sh /root/workspace/QuixNet/ExitControlServers.sh
COPY QuixNet/StartControlServers.sh /root/workspace/QuixNet/StartControlServers.sh
COPY QuixNet/RebuildCmCtlProject.sh /root/workspace/QuixNet/RebuildCmCtlProject.sh


RUN echo "#!/bin/bash\n\
cd /root/workspace/QuixNet\n\
./RebuildCmCtlProject.sh\n\n\
rm -rf /root/workspace/QuixNet/cmake_modules\n\
rm -rf /root/workspace/QuixNet/cache_server/src\n\
rm -rf /root/workspace/QuixNet/master_server/src\n\
rm -rf /root/workspace/QuixNet/servant_server/src\n\
rm -rf /root/workspace/QuixNet/shared/source\n\
rm -rf /root/workspace/QuixNet/shared/include\n\
rm -rf /root/workspace/QuixNet/shared/third_party\n" > /root/workspace/server-setup.sh && chmod a+x /root/workspace/server-setup.sh && ./root/workspace/server-setup.sh


RUN echo "#!/bin/bash\n\n\
/etc/init.d/mysql restart\n\
/etc/init.d/apache2 restart\n\
cd /root/workspace/QuixNet\n\
./StartControlServers.sh Debug\n\n\
trap 'exit 0' 1 2 3 15\n\n\
while :\n\
do\n\
    sleep 3\n\
done\n\n\
echo \"Quit\"\n" > /root/workspace/start.sh && chmod a+x /root/workspace/start.sh