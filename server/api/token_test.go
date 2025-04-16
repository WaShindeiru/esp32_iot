package main

import (
	"server/data"
	"server/internal/util"
	"strings"
	"testing"
	"time"
)

func TestCreateToken(t *testing.T) {
	device := &data.Device{
		Name:      "eps32",
		CreatedAt: time.Now(),
		LastSeen:  time.Now(),
	}

	db, err_ := data.OpenDB()
	util.AssertErrorNil(t, err_)
	repository := data.NewRepository(db)
	device, err := repository.Devices.Insert(device)
	util.AssertErrorNil(t, err)

	token, err := repository.Tokens.New(device.Id)
	util.AssertErrorNil(t, err)

	device_result, err_2 := repository.Devices.GetForToken(token.Plaintext)
	util.AssertErrorNil(t, err_2)

	util.Assert(t, strings.Compare(device.Name, device_result.Name) == 0, "different devices")
}
