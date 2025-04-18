package main

import (
	"fmt"
	"log"
	"net/http"
	"os"
	"server/data"
	"time"
)

type application struct {
	repository data.Repository
	logger     *log.Logger
}

func main() {
	logger := log.New(os.Stdout, "", 0)

	db, err := data.OpenDB()
	if err != nil {
		logger.Fatal(err.Error())
	}
	defer db.Close()
	logger.Print("database connection pool established")

	app := &application{
		repository: data.NewRepository(db),
		logger:     logger,
	}

	err = app.serve()
	if err != nil {
		logger.Fatal(err, nil)
	}
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
