package sitio

import (
	"context"
	"encoding/json"
	"errors"
	"net/http"
	"strconv"
	"strings"

	"superbloques/db"

	"github.com/go-chi/chi/v5"
)

type ctxKey int

const ctxUser ctxKey = 1

func writeJSON(w http.ResponseWriter, status int, v any) {
	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.WriteHeader(status)
	_ = json.NewEncoder(w).Encode(v)
}

func writeError(w http.ResponseWriter, status int, msg string) {
	writeJSON(w, status, map[string]string{"error": msg})
}

func readJSON(r *http.Request, v any) error {
	dec := json.NewDecoder(r.Body)
	if err := dec.Decode(v); err != nil {
		var maxErr *http.MaxBytesError
		if errors.As(err, &maxErr) {
			return errCuerpoGrande
		}
		return err
	}
	return nil
}

var errCuerpoGrande = errors.New("cuerpo demasiado grande")

func parseID(s string) (uint64, error) {
	n, err := strconv.ParseUint(s, 10, 64)
	if err != nil || n == 0 {
		return 0, strconv.ErrSyntax
	}
	return n, nil
}

func urlID(r *http.Request) (uint64, error) {
	return parseID(chi.URLParam(r, "id"))
}

func usuarioDe(r *http.Request) *db.Usuario {
	u, _ := r.Context().Value(ctxUser).(*db.Usuario)
	return u
}

func (s *Server) withSession(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		c, err := r.Cookie(cookieName)
		if err == nil && c.Value != "" {
			sess, err := s.db.Sesion(c.Value)
			if err == nil {
				u, err := s.db.Usuario(sess.UserID)
				if err == nil {
					r = r.WithContext(context.WithValue(r.Context(), ctxUser, u))
				}
			}
		}
		next.ServeHTTP(w, r)
	})
}

func (s *Server) requireAuthJSON(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if usuarioDe(r) == nil {
			writeError(w, http.StatusUnauthorized, "no autenticado")
			return
		}
		next.ServeHTTP(w, r)
	})
}

func maxBytes(n int64) func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			r.Body = http.MaxBytesReader(w, r.Body, n)
			next.ServeHTTP(w, r)
		})
	}
}

func safeNext(s string) string {
	if s == "" || !strings.HasPrefix(s, "/") || strings.HasPrefix(s, "//") || strings.Contains(s, "://") {
		return "/"
	}
	return s
}

func handleJSONErr(w http.ResponseWriter, err error) bool {
	if err == nil {
		return false
	}
	if errors.Is(err, errCuerpoGrande) {
		writeError(w, http.StatusRequestEntityTooLarge, "cuerpo demasiado grande")
		return true
	}
	writeError(w, http.StatusBadRequest, "json invalido")
	return true
}
