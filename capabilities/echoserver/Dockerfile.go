FROM registry.access.redhat.com/ubi8/ubi:latest


EXPOSE 5005

COPY . app/

ENV SERVER_PORT=5005
CMD [ "app/echo" ]
