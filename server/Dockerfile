FROM golang:1.24-alpine AS builder

WORKDIR /app

COPY go.mod go.sum ./

RUN go mod download

COPY . .

RUN CGO_ENABLED=0 GOOS=linux go build -o /app/main ./api

FROM debian:stable-slim

RUN apt-get update && \
    apt-get install -y bash && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/main .

EXPOSE 8080

CMD ["./main"]
