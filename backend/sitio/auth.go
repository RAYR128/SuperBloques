package sitio

import (
	"errors"
	"net/http"
	"regexp"
	"strings"
	"time"
	"unicode/utf8"

	"superbloques/db"
)

const (
	cookieName = "sb_session"
	sessionTTL = 30 * 24 * time.Hour
)

var reNombre = regexp.MustCompile(`^[a-zA-Z0-9_]{3,24}$`)

type credenciales struct {
	Nombre   string `json:"nombre"`
	Password string `json:"password"`
	Display  string `json:"display"`
}

func (s *Server) setSessionCookie(w http.ResponseWriter, token string) {
	http.SetCookie(w, &http.Cookie{
		Name:     cookieName,
		Value:    token,
		Path:     "/",
		MaxAge:   int(sessionTTL.Seconds()),
		HttpOnly: true,
		SameSite: http.SameSiteLaxMode,
		Secure:   s.secureCK,
	})
}

func (s *Server) clearSessionCookie(w http.ResponseWriter) {
	http.SetCookie(w, &http.Cookie{
		Name:     cookieName,
		Value:    "",
		Path:     "/",
		MaxAge:   -1,
		HttpOnly: true,
		SameSite: http.SameSiteLaxMode,
		Secure:   s.secureCK,
	})
}

func (s *Server) emitirSesion(w http.ResponseWriter, u *db.Usuario) error {
	token, err := s.db.CrearSesion(u.ID, sessionTTL)
	if err != nil {
		return err
	}
	s.setSessionCookie(w, token)
	return nil
}

func validarNombre(nombre string) string {
	if !reNombre.MatchString(nombre) {
		return "el usuario debe tener 3-24 caracteres (letras, numeros o _)"
	}
	return ""
}

func validarPassword(password string) string {
	n := utf8.RuneCountInString(password)
	if n < 8 {
		return "la contraseña debe tener al menos 8 caracteres"
	}
	if len(password) > 72 {
		return "la contraseña es demasiado larga"
	}
	return ""
}

func validarDisplay(display string) string {
	display = strings.TrimSpace(display)
	if display == "" {
		return "el nombre visible no puede estar vacio"
	}
	if utf8.RuneCountInString(display) > 40 {
		return "el nombre visible es demasiado largo"
	}
	if strings.ContainsAny(display, "\x00\n\r") {
		return "el nombre visible es invalido"
	}
	return ""
}

func (s *Server) register(w http.ResponseWriter, r *http.Request) {
	var req credenciales
	if err := readJSON(r, &req); handleJSONErr(w, err) {
		return
	}
	req.Nombre = strings.TrimSpace(req.Nombre)
	req.Display = strings.TrimSpace(req.Display)
	if msg := validarNombre(req.Nombre); msg != "" {
		writeError(w, http.StatusBadRequest, msg)
		return
	}
	if msg := validarPassword(req.Password); msg != "" {
		writeError(w, http.StatusBadRequest, msg)
		return
	}
	if req.Display == "" {
		req.Display = req.Nombre
	}
	if msg := validarDisplay(req.Display); msg != "" {
		writeError(w, http.StatusBadRequest, msg)
		return
	}
	u, err := s.db.CrearUsuario(req.Nombre, req.Display, req.Password, false)
	if errors.Is(err, db.ErrDuplicado) {
		writeError(w, http.StatusConflict, "ese usuario ya existe")
		return
	}
	if err != nil {
		writeError(w, http.StatusInternalServerError, "no se pudo crear la cuenta")
		return
	}
	if err := s.emitirSesion(w, u); err != nil {
		writeError(w, http.StatusInternalServerError, "no se pudo iniciar sesion")
		return
	}
	writeJSON(w, http.StatusCreated, u.Publico())
}

func (s *Server) login(w http.ResponseWriter, r *http.Request) {
	var req credenciales
	if err := readJSON(r, &req); handleJSONErr(w, err) {
		return
	}
	req.Nombre = strings.TrimSpace(req.Nombre)
	u, err := s.db.UsuarioPorNombre(req.Nombre)
	if err != nil || !s.db.VerificarPassword(u, req.Password) {
		writeError(w, http.StatusUnauthorized, "usuario o contraseña incorrectos")
		return
	}
	if err := s.emitirSesion(w, u); err != nil {
		writeError(w, http.StatusInternalServerError, "no se pudo iniciar sesion")
		return
	}
	writeJSON(w, http.StatusOK, u.Publico())
}

func (s *Server) logout(w http.ResponseWriter, r *http.Request) {
	c, err := r.Cookie(cookieName)
	if err == nil {
		_ = s.db.BorrarSesion(c.Value)
	}
	s.clearSessionCookie(w)
	w.WriteHeader(http.StatusNoContent)
}

func (s *Server) me(w http.ResponseWriter, r *http.Request) {
	u := usuarioDe(r)
	if u == nil {
		writeError(w, http.StatusUnauthorized, "no autenticado")
		return
	}
	writeJSON(w, http.StatusOK, u.Publico())
}
