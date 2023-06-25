FROM debian
RUN apt update
RUN DEBIAN_FRONTEND=noninteractive apt install firefox-esr mate-system-monitor  git lxde tightvncserver wget   -y
RUN wget https://github.com/novnc/noVNC/archive/refs/tags/v1.2.0.tar.gz
RUN tar -xvf v1.2.0.tar.gz
RUN mkdir  /root/.vnc
RUN echo 'xqx123' | vncpasswd -f > /root/.vnc/passwd
RUN chmod 600 /root/.vnc/passwd
RUN cp /noVNC-1.2.0/vnc.html /noVNC-1.2.0/index.html
RUN echo 'cd /root' >>/start.sh
RUN echo "su root -l -c 'vncserver :2000 ' "  >>/start.sh
RUN echo 'cd /noVNC-1.2.0' >>/start.sh
RUN echo './utils/launch.sh  --vnc localhost:7900 --listen 80 ' >>/start.sh
RUN echo root:quixnet|chpasswd
RUN chmod 755 /start.sh
EXPOSE 80
CMD /start.sh