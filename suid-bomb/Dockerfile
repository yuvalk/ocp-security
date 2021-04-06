FROM centos:centos8
COPY . /app
RUN chown -R root:root /app/
RUN chmod 400 /app/secret.txt
RUN chmod u+s /app/suid-bomb
