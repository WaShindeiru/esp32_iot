package data

import (
	"context"
	"database/sql"
	"errors"
	"fmt"
	"os"
	"time"
)

var ErrNotFound = errors.New("record not present")
var ErrDuplicateName = errors.New("duplicate name")

type Repository struct {
	Devices    DeviceRepository
	SensorData SensorDataRepository
	Tokens     TokenRepository
}

func OpenDB() (*sql.DB, error) {
	connection_string := fmt.Sprintf("postgresql://%s:%s@%s:%s/%s?sslmode=disable",
		os.Getenv("DB_USER"),
		os.Getenv("DB_PASSWORD"),
		os.Getenv("DB_HOST"),
		os.Getenv("DB_PORT"),
		os.Getenv("DB_NAME"))

	//connection_string := "postgresql://postgres:postgres@db:5432/appdb?sslmode=disable"
	//connection_string := "postgresql://iot_user:aha987@localhost:5432/weather_iot"
	db, err := sql.Open("postgres", connection_string)

	if err != nil {
		return nil, err
	}

	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	err = db.PingContext(ctx)
	if err != nil {
		return nil, err
	}

	return db, nil
}

func NewRepository(db *sql.DB) Repository {
	return Repository{
		Devices:    DeviceRepository{db},
		SensorData: SensorDataRepository{Db: db},
		Tokens:     TokenRepository{DB: db},
	}
}
