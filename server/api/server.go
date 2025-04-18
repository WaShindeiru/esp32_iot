package main

import (
	"fmt"
	"log"
	"net/http"
	"server/data"
	"time"
)

type application struct {
	repository data.Repository
	logger     *log.Logger
}

func (app *application) serve() error {
	port := 8080

	server := &http.Server{
		Addr:         fmt.Sprintf(":%d", port),
		Handler:      app.routes(),
		ErrorLog:     app.logger,
		IdleTimeout:  time.Minute,
		ReadTimeout:  10 * time.Second,
		WriteTimeout: 30 * time.Second,
	}

	app.logger.Print("starting server", map[string]string{
		"addr": server.Addr,
	})

	err := server.ListenAndServe()
	app.logger.Print("stopped server", map[string]string{
		"addr":  server.Addr,
		"error": err.Error(),
	})

	return nil
}
