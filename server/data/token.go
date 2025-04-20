package data

import (
	"crypto/rand"
	"crypto/sha256"
	"database/sql"
	"encoding/base32"
)

type Token struct {
	Plaintext string `json:"token"`
	Hash      []byte `json:"-"`
	DeviceId  int64  `json:"-"`
	Scope     string `json:"-"`
}

type TokenRepository struct {
	DB *sql.DB
}

func (m TokenRepository) Insert(token *Token) error {
	query := `
			INSERT INTO tokens (device_id, hash, scope)
			VALUES ($1, $2, $3)`
	_, err := m.DB.Exec(query, token.DeviceId, token.Hash, token.Scope)
	return err
}

func GenerateToken(userId int64) (*Token, error) {
	token := &Token{
		DeviceId: userId,
		Scope:    "default",
	}

	randomBytes := make([]byte, 16)

	_, err := rand.Read(randomBytes)
	if err != nil {
		return nil, err
	}

	token.Plaintext = base32.StdEncoding.WithPadding(base32.NoPadding).EncodeToString(randomBytes)
	hash := sha256.Sum256([]byte(token.Plaintext))

	token.Hash = hash[:]

	return token, nil
}

func (m TokenRepository) New(userID int64) (*Token, error) {
	token, err := GenerateToken(userID)
	if err != nil {
		return nil, err
	}

	err = m.Insert(token)
	if err != nil {
		return nil, err
	}

	return token, nil
}

func (m TokenRepository) NewPlaceholder(userId int64) (*Token, error) {
	token := &Token{
		DeviceId: userId,
		Scope:    "default",
	}

	bytes := make([]byte, 16)
	for i := range bytes {
		bytes[i] = byte(i)
	}

	token.Plaintext = base32.StdEncoding.WithPadding(base32.NoPadding).EncodeToString(bytes)
	hash := sha256.Sum256([]byte(token.Plaintext))

	token.Hash = hash[:]

	err := m.Insert(token)
	if err != nil {
		return nil, err
	}

	return token, nil
}
