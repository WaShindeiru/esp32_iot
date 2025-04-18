package main

import (
	"errors"
	"net/http"
	"server/data"
	"time"
)

func (app *application) RegisterDeviceHandler(w http.ResponseWriter, r *http.Request) {
	var input struct {
		Name     string `json:"name"`
		Password string `json:"password"`
	}

	err := app.readJSON(w, r, &input)

	if err != nil {
		app.badRequestResponse(w, r, err)
		return
	}

	device := &data.Device{
		Name:      input.Name,
		CreatedAt: time.Now(),
		LastSeen:  time.Now(),
	}

	err = device.Password.Set(input.Password)

	if err != nil {
		app.badRequestResponse(w, r, err)
		return
	}

	device, err = app.repository.Devices.Insert(device)
	if err != nil {
		switch {
		case errors.Is(err, data.ErrDuplicateName):
			app.badRequestResponse(w, r, err)
		default:
			app.serverErrorResponse(w, r, err)
		}

		return
	}

	var token *data.Token
	token, err = app.repository.Tokens.New(device.Id)
	if err != nil {
		app.serverErrorResponse(w, r, err)
		return
	}

	err = app.writeJSON(w, http.StatusCreated, envelope{"device": device, "token": token}, nil)
	if err != nil {
		app.serverErrorResponse(w, r, err)
	}
}
