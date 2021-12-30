FROM alpine
RUN apk update && apk add bash
WORKDIR /app
COPY argsprinter .

CMD ["/app/argsprinter","1","2","3"]
