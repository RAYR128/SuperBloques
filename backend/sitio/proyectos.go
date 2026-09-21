package sitio

import (
	"bytes"
	"encoding/base64"
	"encoding/json"
	"errors"
	"image"
	"image/color"
	"image/jpeg"
	"net/http"
	"strconv"
	"strings"
	"unicode/utf8"

	"superbloques/db"
)

const (
	thumbMaxBytes = 256 * 1024
	nombreMax     = 80
)

var jpegNegro []byte

func init() {
	img := image.NewRGBA(image.Rect(0, 0, 1, 1))
	img.Set(0, 0, color.Black)
	var buf bytes.Buffer
	_ = jpeg.Encode(&buf, img, &jpeg.Options{Quality: 50})
	jpegNegro = buf.Bytes()
}

type proyectoBody struct {
	Nombre    string          `json:"nombre"`
	Proyecto  json.RawMessage `json:"proyecto"`
	Thumbnail string          `json:"thumbnail"`
}

type proyectoLista struct {
	ID           uint64 `json:"id"`
	Nombre       string `json:"nombre"`
	OwnerID      uint64 `json:"ownerId"`
	OwnerNombre  string `json:"ownerNombre"`
	OwnerDisplay string `json:"ownerDisplay"`
	Actualizado  string `json:"actualizado"`
	Thumb        string `json:"thumb"`
}

type proyectoDetalle struct {
	ID          uint64          `json:"id"`
	Nombre      string          `json:"nombre"`
	OwnerID     uint64          `json:"ownerId"`
	OwnerNombre string          `json:"ownerNombre"`
	Actualizado string          `json:"actualizado"`
	Proyecto    json.RawMessage `json:"proyecto"`
}

func validarNombreProyecto(nombre string) string {
	nombre = strings.TrimSpace(nombre)
	if nombre == "" {
		return "el nombre del proyecto no puede estar vacio"
	}
	if utf8.RuneCountInString(nombre) > nombreMax {
		return "el nombre del proyecto es demasiado largo"
	}
	return ""
}

func validarJSONProyecto(raw json.RawMessage) error {
	if len(bytes.TrimSpace(raw)) == 0 {
		return errors.New("falta el proyecto")
	}
	var obj map[string]json.RawMessage
	if err := json.Unmarshal(raw, &obj); err != nil {
		return errors.New("el proyecto no es un objeto json")
	}
	for _, k := range []string{"Escenas", "Objetos"} {
		v, ok := obj[k]
		if !ok || len(bytes.TrimSpace(v)) == 0 {
			continue
		}
		var m map[string]json.RawMessage
		if err := json.Unmarshal(v, &m); err != nil {
			return errors.New(k + " debe ser un objeto")
		}
	}
	return nil
}

func decodeThumb(s string) ([]byte, error) {
	s = strings.TrimSpace(s)
	if s == "" {
		return nil, nil
	}
	payload := s
	if strings.HasPrefix(s, "data:") {
		comma := strings.Index(s, ",")
		if comma < 0 {
			return nil, errors.New("thumbnail invalido")
		}
		meta := strings.ToLower(s[:comma])
		if !strings.Contains(meta, "image/jpeg") && !strings.Contains(meta, "image/jpg") {
			return nil, errors.New("el thumbnail debe ser jpeg")
		}
		payload = s[comma+1:]
		data, err := base64.StdEncoding.DecodeString(payload)
		if err != nil {
			return nil, errors.New("thumbnail invalido")
		}
		return checkJPEG(data)
	}
	if strings.HasPrefix(s, "\xff\xd8") {
		return checkJPEG([]byte(s))
	}
	data, err := base64.StdEncoding.DecodeString(payload)
	if err != nil {
		return nil, errors.New("thumbnail invalido")
	}
	return checkJPEG(data)
}

func checkJPEG(data []byte) ([]byte, error) {
	if len(data) < 3 || data[0] != 0xff || data[1] != 0xd8 || data[2] != 0xff {
		return nil, errors.New("el thumbnail debe ser jpeg")
	}
	if len(data) > thumbMaxBytes {
		return nil, errors.New("el thumbnail es demasiado grande")
	}
	return data, nil
}

func ownerNombres(d *db.DB, ownerID uint64) (nombre, display string) {
	u, err := d.Usuario(ownerID)
	if err != nil {
		return "", ""
	}
	return u.Nombre, u.Display
}

func metaALista(d *db.DB, p db.ProyectoMeta) proyectoLista {
	nombre, display := ownerNombres(d, p.OwnerID)
	return proyectoLista{
		ID:           p.ID,
		Nombre:       p.Nombre,
		OwnerID:      p.OwnerID,
		OwnerNombre:  nombre,
		OwnerDisplay: display,
		Actualizado:  p.Actualizado.UTC().Format("2006-01-02T15:04:05Z07:00"),
		Thumb:        "/project/" + strconv.FormatUint(p.ID, 10) + "/thumb",
	}
}

func puedeEditar(u *db.Usuario, p *db.ProyectoMeta) bool {
	if u == nil || p == nil {
		return false
	}
	return u.Admin || u.ID == p.OwnerID
}

func (s *Server) createProject(w http.ResponseWriter, r *http.Request) {
	u := usuarioDe(r)
	var req proyectoBody
	if err := readJSON(r, &req); handleJSONErr(w, err) {
		return
	}
	req.Nombre = strings.TrimSpace(req.Nombre)
	if msg := validarNombreProyecto(req.Nombre); msg != "" {
		writeError(w, http.StatusBadRequest, msg)
		return
	}
	if err := validarJSONProyecto(req.Proyecto); err != nil {
		writeError(w, http.StatusBadRequest, err.Error())
		return
	}
	thumb, err := decodeThumb(req.Thumbnail)
	if err != nil {
		writeError(w, http.StatusBadRequest, err.Error())
		return
	}
	p, err := s.db.CrearProyecto(u.ID, req.Nombre, []byte(req.Proyecto), thumb)
	if err != nil {
		writeError(w, http.StatusInternalServerError, "no se pudo crear el proyecto")
		return
	}
	writeJSON(w, http.StatusCreated, map[string]any{"id": p.ID, "nombre": p.Nombre})
}

func (s *Server) updateProject(w http.ResponseWriter, r *http.Request) {
	id, err := urlID(r)
	if err != nil {
		writeError(w, http.StatusBadRequest, "id invalido")
		return
	}
	p, err := s.db.Proyecto(id)
	if errors.Is(err, db.ErrNoEncontrado) {
		writeError(w, http.StatusNotFound, "proyecto no encontrado")
		return
	}
	if err != nil {
		writeError(w, http.StatusInternalServerError, "error al leer proyecto")
		return
	}
	if !puedeEditar(usuarioDe(r), p) {
		writeError(w, http.StatusForbidden, "no tenes permiso para editar este proyecto")
		return
	}
	var req proyectoBody
	if err := readJSON(r, &req); handleJSONErr(w, err) {
		return
	}
	if err := validarJSONProyecto(req.Proyecto); err != nil {
		writeError(w, http.StatusBadRequest, err.Error())
		return
	}
	var nombre *string
	if strings.TrimSpace(req.Nombre) != "" {
		n := strings.TrimSpace(req.Nombre)
		if msg := validarNombreProyecto(n); msg != "" {
			writeError(w, http.StatusBadRequest, msg)
			return
		}
		nombre = &n
	}
	thumb, err := decodeThumb(req.Thumbnail)
	if err != nil {
		writeError(w, http.StatusBadRequest, err.Error())
		return
	}
	updated, err := s.db.ActualizarProyecto(id, nombre, []byte(req.Proyecto), thumb)
	if err != nil {
		writeError(w, http.StatusInternalServerError, "no se pudo actualizar el proyecto")
		return
	}
	writeJSON(w, http.StatusOK, map[string]any{"id": updated.ID, "nombre": updated.Nombre})
}

func (s *Server) deleteProject(w http.ResponseWriter, r *http.Request) {
	id, err := urlID(r)
	if err != nil {
		writeError(w, http.StatusBadRequest, "id invalido")
		return
	}
	p, err := s.db.Proyecto(id)
	if errors.Is(err, db.ErrNoEncontrado) {
		writeError(w, http.StatusNotFound, "proyecto no encontrado")
		return
	}
	if err != nil {
		writeError(w, http.StatusInternalServerError, "error al leer proyecto")
		return
	}
	if !puedeEditar(usuarioDe(r), p) {
		writeError(w, http.StatusForbidden, "no tenes permiso para borrar este proyecto")
		return
	}
	if err := s.db.BorrarProyecto(id); err != nil {
		writeError(w, http.StatusInternalServerError, "no se pudo borrar el proyecto")
		return
	}
	w.WriteHeader(http.StatusNoContent)
}

func (s *Server) getProject(w http.ResponseWriter, r *http.Request) {
	id, err := urlID(r)
	if err != nil {
		writeError(w, http.StatusBadRequest, "id invalido")
		return
	}
	p, err := s.db.Proyecto(id)
	if errors.Is(err, db.ErrNoEncontrado) {
		writeError(w, http.StatusNotFound, "proyecto no encontrado")
		return
	}
	if err != nil {
		writeError(w, http.StatusInternalServerError, "error al leer proyecto")
		return
	}
	data, err := s.db.ProyectoDatos(id)
	if err != nil {
		writeError(w, http.StatusInternalServerError, "error al leer el json del proyecto")
		return
	}
	ownerNombre, _ := ownerNombres(s.db, p.OwnerID)
	writeJSON(w, http.StatusOK, proyectoDetalle{
		ID:          p.ID,
		Nombre:      p.Nombre,
		OwnerID:     p.OwnerID,
		OwnerNombre: ownerNombre,
		Actualizado: p.Actualizado.UTC().Format("2006-01-02T15:04:05Z07:00"),
		Proyecto:    json.RawMessage(data),
	})
}

func (s *Server) listProjects(w http.ResponseWriter, r *http.Request) {
	list, err := s.db.ProyectosRecientes(5)
	if err != nil {
		writeError(w, http.StatusInternalServerError, "error al listar proyectos")
		return
	}
	out := make([]proyectoLista, 0, len(list))
	for _, p := range list {
		out = append(out, metaALista(s.db, p))
	}
	writeJSON(w, http.StatusOK, out)
}

func (s *Server) listUserProjects(w http.ResponseWriter, r *http.Request) {
	id, err := urlID(r)
	if err != nil {
		writeError(w, http.StatusBadRequest, "id invalido")
		return
	}
	if _, err := s.db.Usuario(id); errors.Is(err, db.ErrNoEncontrado) {
		writeError(w, http.StatusNotFound, "usuario no encontrado")
		return
	} else if err != nil {
		writeError(w, http.StatusInternalServerError, "error al leer usuario")
		return
	}
	list, err := s.db.ProyectosDeUsuario(id)
	if err != nil {
		writeError(w, http.StatusInternalServerError, "error al listar proyectos")
		return
	}
	out := make([]proyectoLista, 0, len(list))
	for _, p := range list {
		out = append(out, metaALista(s.db, p))
	}
	writeJSON(w, http.StatusOK, out)
}

func (s *Server) projectThumb(w http.ResponseWriter, r *http.Request) {
	id, err := urlID(r)
	if err != nil {
		http.NotFound(w, r)
		return
	}
	if _, err := s.db.Proyecto(id); errors.Is(err, db.ErrNoEncontrado) {
		http.NotFound(w, r)
		return
	} else if err != nil {
		http.Error(w, "error", http.StatusInternalServerError)
		return
	}
	data, err := s.db.Thumb(id)
	if errors.Is(err, db.ErrNoEncontrado) || len(data) == 0 {
		data = jpegNegro
	} else if err != nil {
		http.Error(w, "error", http.StatusInternalServerError)
		return
	}
	w.Header().Set("Content-Type", "image/jpeg")
	w.Header().Set("Cache-Control", "public, max-age=60")
	w.WriteHeader(http.StatusOK)
	_, _ = w.Write(data)
}
