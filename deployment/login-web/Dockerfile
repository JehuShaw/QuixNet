FROM debian

RUN apt-get update —fix-missing && apt-get install -y net-tools —fix-missing && apt-get install -y inetutils-ping —fix-missing && apt-get install -y telnet —fix-missing && apt-get install -y vim —fix-missing && apt-get install -y mysql-server —fix-missing 
RUN echo "[client] \nport  = 3306\n[mysqld] \nport = 3306\n" >> /etc/mysql/my.cnf && /etc/init.d/mysql restart
RUN apt-get install -y apache2 —fix-missing && apt-get install -y libapache2-mod-php —fix-missing && apt-get install -y php-mysql —fix-missing 
RUN echo "ServerName 127.0.0.1:80" >> /etc/apache2/apache2.conf && /etc/init.d/apache2 restart

COPY login-web /root/workspace/login-web

RUN echo "#!/bin/bash\n\n/etc/init.d/mysql restart\ncd /root/workspace/login-web/sql/\nmysql -h127.0.0.1 -P3306 -uroot < create_database.sql;\nmysql -h127.0.0.1 -P3306 -uroot -Daccount < account.sql;" > /root/workspace/login-web/sql/loadsql.sh && chmod a+x /root/workspace/login-web/sql/loadsql.sh && ./root/workspace/login-web/sql/loadsql.sh
COPY login-web/php /var/www/html/quixnet

RUN echo "#!/bin/bash\n\n\
/etc/init.d/mysql restart\n\
/etc/init.d/apache2\n\n\
trap 'exit 0' 1 2 3 15\n\n\
while :\n\
do\n\
    sleep 3\n\
done\n\n\
echo \"Quit\"\n" > /root/workspace/start.sh && chmod a+x /root/workspace/start.sh