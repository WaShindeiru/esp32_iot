package main

import (
	"net/http"
	"server/data"
	"time"
)

func (app *application) getSensorData(w http.ResponseWriter, r *http.Request) {
	device := r.Context().Value("device").(*data.Device)
	if device == nil {
		panic("User not authenticated")
	}

	sensor_data, err := app.repository.SensorData.GetAllForDevice(device.Id)

	if err != nil {
		app.serverErrorResponse(w, r, err)
		return
	}

	err = app.writeJSON(w, http.StatusOK, envelope{"sensor data": sensor_data}, nil)
	if err != nil {
		app.serverErrorResponse(w, r, err)
	}
}

func (app *application) postSensorData(w http.ResponseWriter, r *http.Request) {
	device := r.Context().Value("device").(*data.Device)
	if device == nil {
		panic("User not authenticated")
	}

	var input struct {
		Humidity    float64 `json:"humidity"`
		Temperature float64 `json:"temperature"`
	}

	err := app.readJSON(w, r, &input)
	if err != nil {
		app.badRequestResponse(w, r, err)
		return
	}

	sensor_data := &data.SensorData{
		Time:        time.Now(),
		Humidity:    input.Humidity,
		Temperature: input.Temperature,
		Device_id:   device.Id,
	}

	sensor_data, err = app.repository.SensorData.Insert(sensor_data)
	if err != nil {
		app.serverErrorResponse(w, r, err)
		return
	}

	err = app.writeJSON(w, http.StatusCreated, envelope{"sensor_data": sensor_data}, nil)
	if err != nil {
		app.serverErrorResponse(w, r, err)
	}
}
