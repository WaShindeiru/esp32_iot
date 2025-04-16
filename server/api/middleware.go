package main

import (
	"context"
	"errors"
	"net/http"
	"server/data"
	"strings"
)

func (app *application) authenticate(next http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		w.Header().Add("Vary", "Authorization")
		authorizationHeader := r.Header.Get("Authorization")

		if authorizationHeader == "" {
			app.invalidAuthenticationTokenResponse(w, r)
			return
		}

		headerParts := strings.Split(authorizationHeader, " ")
		if len(headerParts) != 2 || headerParts[0] != "Bearer" {
			app.invalidAuthenticationTokenResponse(w, r)
			return
		}

		token := headerParts[1]

		device, err := app.repository.Devices.GetForToken(token)
		if err != nil {
			switch {
			case errors.Is(err, data.ErrNotFound):
				app.invalidAuthenticationTokenResponse(w, r)
			default:
				app.serverErrorResponse(w, r, err)
			}
			return
		}

		ctx := context.WithValue(r.Context(), "device", device)
		r = r.WithContext(ctx)

		next.ServeHTTP(w, r)
	}
}
