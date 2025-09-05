# Use the official Ubuntu base image
FROM ubuntu:latest
 
# Set time zone (optional)
ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
 
RUN apt-get update --fix-missing && apt-get install -y net-tools --fix-missing && apt-get install -y telnet --fix-missing && 
apt-get install -y vim --fix-missing && apt-get install -y mysql-server --fix-missing

RUN echo "[client] \npoort   = 3309\n[mysqld]\nport    = 3309\n" >> /etc/mysql/my.cnf && /etc/init.d/mysql restart

RUN apt-get install -y apache2 --fix-missing && apt-get install -y php libapache2-mod-php --fix-missing && apt-get install -y php-mysql --fix-missing
RUN echo "ServerName 127.0.0.1:80" >> /etc/apache2/apache2.conf && /etc/init.d/apache2 restart

COPY web/sql/* /root/workspace/QuixNet/web/sql
COPY web/api /root/workspace/QuixNet/web/api

RUN echo "#!/bin/bash\n\n/etc/init.d/mysql restart\ncd /root/workspace/QuixNet/web/sql/\nmysql -h127.0.0.1 -p3309 -uroot < create_database.sql;\n
mysql -h127.0.0.1 -p3309 -uroot -Daccount < account.sql;" > /root/workspace/QuixNet/web/sql/loadsql.sh && chmod a+x /root/workspace/QuixNet/web/sql/loadsql.sh && ./root/workspace/QuixNet/web/sql/loadsql.sh

COPY web/api /var/www/html/QuixNet-web/api

RUN cat /root/workspace/QuixNet/web/api/game/config.php | sed 's/$agentIp.*/$agentIp = \"127.0.0.1:5551\";/g' | sed 's/^M//g' > /var/www/html/QuixNet-web/api/game/config.php

RUN cat /root/workspace/QuixNet/web/api/game/config.php | sed 's/^M//g' > /var/www/html/QuixNet-web/api/game/config.php

RUN echo "<?php\n\
\t// mysql host\n\
\t\$account_db_host = \"127.0.0.1\";\n\
\t// mysql user name\n\
\t\$account_db_username = \"root\";\n\
\t// mysql password\n\
\t\$account_db_password = \"\";\n\
\t// default database\n\
\t\$account_db_database = \"account\";\n\
\t// port\n\
\t\$account_db_port = 3309;\n\
?>" > /var/www/html/QuixNet-web/db_config.php


RUN echo "#!/bin/bash\n\n\
/etc/init.d/mysql restart\n\
/etc/init.d/apache2 restart\n\n\
trap 'exit 0' 1 2 3 15\n\n\
while :\n\
do\n\
    sleep 3\n\
done\n\n\
echo \"Quit\"\n" > /root/workspace/start.sh && chmod a+x /root/workspace/start.sh