package main

import (
	"fmt"
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

	token, err := app.registerDeviceHelper("esp32_2", "temp")

	if err != nil {
		logger.Fatal(err.Error())
	} else {
		// token: AAAQEAYEAUDAOCAJBIFQYDIOB4
		logger.Print(fmt.Sprintf("token: %s", token.Plaintext))
	}

	err = app.serve()
	if err != nil {
		logger.Fatal(err, nil)
	}
}
