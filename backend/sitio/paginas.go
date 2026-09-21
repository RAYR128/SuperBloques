package sitio

import (
	"errors"
	"net/http"
	"os"
	"path/filepath"

	"superbloques/db"
)

func (s *Server) serveSitio(name string) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, filepath.Join(s.sitioDir, name))
	}
}

func (s *Server) notFoundPage(w http.ResponseWriter, r *http.Request) {
	path := filepath.Join(s.sitioDir, "404.html")
	body, err := os.ReadFile(path)
	if err != nil {
		http.NotFound(w, r)
		return
	}
	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	w.WriteHeader(http.StatusNotFound)
	_, _ = w.Write(body)
}

func (s *Server) pageLogin(w http.ResponseWriter, r *http.Request) {
	if usuarioDe(r) != nil {
		http.Redirect(w, r, safeNext(r.URL.Query().Get("next")), http.StatusSeeOther)
		return
	}
	s.serveSitio("login.html")(w, r)
}

func (s *Server) pageRegister(w http.ResponseWriter, r *http.Request) {
	if usuarioDe(r) != nil {
		http.Redirect(w, r, safeNext(r.URL.Query().Get("next")), http.StatusSeeOther)
		return
	}
	s.serveSitio("register.html")(w, r)
}

func (s *Server) pageAccount(w http.ResponseWriter, r *http.Request) {
	if usuarioDe(r) == nil {
		http.Redirect(w, r, "/login?next=/account", http.StatusSeeOther)
		return
	}
	s.serveSitio("account.html")(w, r)
}

func (s *Server) pageProject(w http.ResponseWriter, r *http.Request) {
	id, err := urlID(r)
	if err != nil {
		s.notFoundPage(w, r)
		return
	}
	_, err = s.db.Proyecto(id)
	if errors.Is(err, db.ErrNoEncontrado) {
		s.notFoundPage(w, r)
		return
	}
	if err != nil {
		http.Error(w, "error al leer proyecto", http.StatusInternalServerError)
		return
	}
	http.ServeFile(w, r, filepath.Join(s.editorDir, "index.html"))
}
