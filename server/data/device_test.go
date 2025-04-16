package data

import (
	"testing"
	"time"
)

func TestInsertSelect(t *testing.T) {
	device := &Device{
		Name:      "eps32",
		CreatedAt: time.Now(),
		LastSeen:  time.Now(),
	}

	err := device.Password.Set("monke")
	if err != nil {
		t.Fatal("can't set password")
	}

	db, err := OpenDB()
	if err != nil {
		t.Fatal(err)
	}

	repository := DeviceRepository{db}
	_, err = repository.Insert(device)
	if err != nil {
		t.Fatal(err)
	}

	result_device, err := repository.GetByName(device.Name)
	if err != nil {
		t.Fatal(err)
	}

	password_match, _ := result_device.Password.Matches(*device.Password.plaintext)
	name_match := result_device.Name == device.Name
	created_at_match := result_device.CreatedAt.Truncate(time.Second).Equal(device.CreatedAt.Truncate(time.Second))
	last_seen_match := result_device.LastSeen.Truncate(time.Second).Equal(device.LastSeen.Truncate(time.Second))
	if !name_match || !created_at_match || !last_seen_match || !password_match {
		t.Fatal("wrong result")
	}
}
