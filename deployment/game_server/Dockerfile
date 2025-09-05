# Use the official Ubuntu base image
FROM ubuntu:latest
 
# Set time zone (optional)
ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
 
RUN apt-get update --fix-missing && apt-get install -y apt-utils --fix-missing && apt-get install -y net-tools --fix-missing && apt-get install -y inetutils-ping --fix-missing &&
apt-get install -y telnet --fix-missing && apt-get install -y vim --fix-missing && apt-get install -y libssh2-1-dev --fix-missing && apt-get install -y gdb --fix-missing && 
apt-get install -y screen --fix-missing && apt-get install -y libmysqlclient-dev --fix-missing


COPY QuixNet /root/workspace/QuixNet


RUN echo "#!/bin/bash\n\
cd /root/workspace/QuixNet\n\
./RebuildCmProject.sh\n\n\
rm -rf /root/workspace/QuixNet/agent_server/src\n\
mv /root/workspace/QuixNet/game_server/src /root/workspace/QuixNet/game_server/src-backup\n\
mkdir /root/workspace/QuixNet/game_server/src\n\
cp -rf /root/workspace/QuixNet/game_server/src-backup/protocol /root/workspace/QuixNet/game_server/src/\n\
rm -rf /root/workspace/QuixNet/game_server/src-backup\n\
rm -rf /root/workspace/QuixNet/shared/source\n\
rm -rf /root/workspace/QuixNet/shared/include\n\
rm -rf /root/workspace/QuixNet/shared/third_party\n\
rm -rf /root/workspace/QuixNet/tools\n\
rm -rf /root/workspace/QuixNet/deployment\n" > /root/workspace/server-setup.sh && chmod a+x /root/workspace/server-setup.sh && ./root/workspace/server-setup.sh


RUN echo "#!/bin/bash\n\n\
cd /root/workspace/QuixNet\n\
./StartServers.sh Debug\n\n\
trap 'exit 0' 1 2 3 15\n\n\
while :\n\
do\n\
    sleep 3\n\
done\n\n\
echo \"Quit\"\n" > /root/workspace/start.sh && chmod a+x /root/workspace/start.sh