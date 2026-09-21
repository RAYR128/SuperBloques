package sitio

import (
	"errors"
	"net/http"
	"strings"

	"superbloques/db"
)

type cuentaReq struct {
	Display        string `json:"display"`
	PasswordActual string `json:"passwordActual"`
	PasswordNueva  string `json:"passwordNueva"`
}

func (s *Server) getUser(w http.ResponseWriter, r *http.Request) {
	id, err := urlID(r)
	if err != nil {
		writeError(w, http.StatusBadRequest, "id invalido")
		return
	}
	u, err := s.db.Usuario(id)
	if errors.Is(err, db.ErrNoEncontrado) {
		writeError(w, http.StatusNotFound, "usuario no encontrado")
		return
	}
	if err != nil {
		writeError(w, http.StatusInternalServerError, "error al leer usuario")
		return
	}
	writeJSON(w, http.StatusOK, u.Publico())
}

func (s *Server) updateAccount(w http.ResponseWriter, r *http.Request) {
	u := usuarioDe(r)
	var req cuentaReq
	if err := readJSON(r, &req); handleJSONErr(w, err) {
		return
	}
	if !s.db.VerificarPassword(u, req.PasswordActual) {
		writeError(w, http.StatusUnauthorized, "contraseña actual incorrecta")
		return
	}
	if strings.TrimSpace(req.Display) != "" {
		display := strings.TrimSpace(req.Display)
		if msg := validarDisplay(display); msg != "" {
			writeError(w, http.StatusBadRequest, msg)
			return
		}
		u.Display = display
	}
	if req.PasswordNueva != "" {
		if msg := validarPassword(req.PasswordNueva); msg != "" {
			writeError(w, http.StatusBadRequest, msg)
			return
		}
		if err := s.db.CambiarPassword(u, req.PasswordNueva); err != nil {
			writeError(w, http.StatusInternalServerError, "no se pudo cambiar la contraseña")
			return
		}
	} else if err := s.db.GuardarUsuario(u); err != nil {
		writeError(w, http.StatusInternalServerError, "no se pudo guardar")
		return
	}
	fresh, err := s.db.Usuario(u.ID)
	if err != nil {
		writeJSON(w, http.StatusOK, u.Publico())
		return
	}
	writeJSON(w, http.StatusOK, fresh.Publico())
}
