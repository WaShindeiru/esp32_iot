package main

import (
	"log"
	"os"
	"server/data"
)

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
