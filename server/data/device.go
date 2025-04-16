package data

import (
	"context"
	"crypto/sha256"
	"database/sql"
	"errors"
	"golang.org/x/crypto/bcrypt"
	"time"

	_ "github.com/lib/pq"
)

type password struct {
	plaintext *string
	hash      []byte
}

type Device struct {
	Id        int64
	Name      string
	Password  password
	CreatedAt time.Time
	LastSeen  time.Time
}

func (p *password) Set(plaintextPassword string) error {
	hash, err := bcrypt.GenerateFromPassword([]byte(plaintextPassword), 12)
	if err != nil {
		return err
	}

	p.plaintext = &plaintextPassword
	p.hash = hash

	return nil
}

func (p *password) Matches(plainTextPassword string) (bool, error) {
	err := bcrypt.CompareHashAndPassword(p.hash, []byte(plainTextPassword))

	if err != nil {
		switch {
		case errors.Is(err, bcrypt.ErrMismatchedHashAndPassword):
			return false, nil
		default:
			return false, err
		}
	}

	return true, nil
}

type DeviceModel interface {
	Insert(user *Device) (*Device, error)
	RemoveByName(name string) error
	DeleteByName(name string) error
	GetByName(name string) (*Device, error)
}

type DeviceRepository struct {
	Db *sql.DB
}

func (d DeviceRepository) GetByName(name string) (*Device, error) {
	query := `
	select id, name, password_hash, created_at, last_seen from devices where name = $1;
	`

	result_device := &Device{}
	err := d.Db.QueryRow(query, name).Scan(&result_device.Id, &result_device.Name, &result_device.Password.hash,
		&result_device.CreatedAt, &result_device.LastSeen)
	if err != nil {
		return nil, err
	}

	return result_device, nil
}

func (d DeviceRepository) RemoveByName(name string) error {
	query := "DELETE FROM devices WHERE name = $1"
	_, err := d.Db.Exec(query, name)

	return err
}

func (d DeviceRepository) Insert(user *Device) (*Device, error) {
	query := `
	insert into devices (name, password_hash, created_at, last_seen) values ($1, $2, $3, $4);
	`
	ctx := context.Background()
	_, err := d.Db.ExecContext(ctx, query, user.Name, user.Password.hash, user.CreatedAt, user.LastSeen)
	if err != nil {
		return nil, err
	}

	query_2 := `
	select id, name, password_hash, created_at, last_seen from devices where name = $1;
	`
	result_device := &Device{}

	err_3 := d.Db.QueryRow(query_2, user.Name).Scan(&result_device.Id, &result_device.Name, &result_device.Password.hash,
		&result_device.CreatedAt, &result_device.LastSeen)

	if err_3 != nil {
		switch {
		case err_3.Error() == `pq: duplicate key value violates unique constraint "users_email_key"`:
			return nil, ErrDuplicateName
		default:
			return nil, err_3
		}
	}

	return result_device, nil
}

func (d DeviceRepository) GetForToken(tokenPlainText string) (*Device, error) {
	tokenHash := sha256.Sum256([]byte(tokenPlainText))
	query := `SELECT devices.id, devices.created_at, devices.name, devices.password_hash, devices.last_seen FROM devices
		INNER JOIN tokens
		ON devices.id = tokens.device_id
		WHERE tokens.hash = $1`

	var device Device

	err := d.Db.QueryRow(query, tokenHash[:]).Scan(
		&device.Id,
		&device.CreatedAt,
		&device.Name,
		&device.Password.hash,
		&device.LastSeen,
	)

	if err != nil {
		switch {
		case errors.Is(err, sql.ErrNoRows):
			return nil, ErrNotFound
		default:
			return nil, err
		}
	}

	return &device, nil
}
