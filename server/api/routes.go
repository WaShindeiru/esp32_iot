package main

import (
	"net/http"

	"github.com/julienschmidt/httprouter"
)

func (app *application) routes() http.Handler {
	router := httprouter.New()

	router.HandlerFunc(http.MethodGet, "/sensorData", app.authenticate(app.getSensorData))
	router.HandlerFunc(http.MethodPost, "/sensorData", app.authenticate(app.postSensorData))

	router.HandlerFunc(http.MethodPost, "/device", app.RegisterDeviceHandler)

	return router
}
