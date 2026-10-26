package sitio

import (
	"bytes"
	"encoding/base64"
	"encoding/json"
	"image"
	"image/color"
	"image/jpeg"
	"net/http"
	"strconv"
	"strings"
	"testing"
	"time"
)

func jpegDataURL(t *testing.T) string {
	t.Helper()
	img := image.NewRGBA(image.Rect(0, 0, 2, 2))
	img.Set(0, 0, color.RGBA{R: 255, A: 255})
	var buf bytes.Buffer
	if err := jpeg.Encode(&buf, img, &jpeg.Options{Quality: 50}); err != nil {
		t.Fatal(err)
	}
	return "data:image/jpeg;base64," + base64.StdEncoding.EncodeToString(buf.Bytes())
}

func cuerpoProyecto(nombre, proyecto, thumb string) map[string]any {
	m := map[string]any{
		"nombre":   nombre,
		"proyecto": json.RawMessage(proyecto),
	}
	if thumb != "" {
		m["thumbnail"] = thumb
	}
	return m
}

func (e *entorno) crearProyecto(c *http.Client, nombre, proyecto, thumb string) map[string]any {
	e.t.Helper()
	resp := e.postJSON(c, "/project", cuerpoProyecto(nombre, proyecto, thumb))
	exigirStatus(e.t, resp, http.StatusCreated)
	var out map[string]any
	decodificarJSON(e.t, resp, &out)
	return out
}

func TestCrearProyectoHTTP(t *testing.T) {
	e := nuevoEntorno(t)
	sin := e.postJSON(e.anonimo(), "/project", map[string]string{
		"nombre":   "Juego",
		"proyecto": proyectoMinimo,
	})
	exigirStatus(t, sin, http.StatusUnauthorized)
	_ = leerCuerpo(t, sin)

	e.registrar(e.authed, "ana", "Ana", "clave-123")
	creado := e.crearProyecto(e.authed, "Juego", proyectoMinimo, jpegDataURL(t))
	if creado["nombre"] != "Juego" || creado["id"] != float64(1) {
		t.Fatalf("creado = %+v", creado)
	}

	ver := e.hacer(e.anonimo(), http.MethodGet, "/api/project/1", "")
	exigirStatus(t, ver, http.StatusOK)
	var det struct {
		Nombre   string          `json:"nombre"`
		Proyecto json.RawMessage `json:"proyecto"`
	}
	decodificarJSON(t, ver, &det)
	if det.Nombre != "Juego" {
		t.Fatalf("nombre = %s", det.Nombre)
	}
	if strings.TrimSpace(string(det.Proyecto)) != proyectoMinimo {
		t.Fatalf("proyecto = %s", det.Proyecto)
	}

	thumb := e.hacer(e.anonimo(), http.MethodGet, "/project/1/thumb", "")
	exigirStatus(t, thumb, http.StatusOK)
	if ct := thumb.Header.Get("Content-Type"); ct != "image/jpeg" {
		t.Fatalf("content-type = %q", ct)
	}
	cuerpo := leerCuerpo(t, thumb)
	if len(cuerpo) < 3 || cuerpo[0] != 0xff || cuerpo[1] != 0xd8 {
		t.Fatalf("no es jpeg: %v", cuerpo[:min(8, len(cuerpo))])
	}
}

func TestCrearProyectoThumbYNombre(t *testing.T) {
	e := nuevoEntorno(t)
	e.registrar(e.authed, "ana", "Ana", "clave-123")

	png := "data:image/png;base64," + base64.StdEncoding.EncodeToString([]byte("png"))
	malo := e.postJSON(e.authed, "/project", cuerpoProyecto("Juego", proyectoMinimo, png))
	exigirStatus(t, malo, http.StatusBadRequest)
	if msg := errorJSON(t, malo); msg != "el thumbnail debe ser jpeg" {
		t.Fatalf("png: %q", msg)
	}

	noJPEG := base64.StdEncoding.EncodeToString([]byte("no-es-jpeg"))
	otro := e.postJSON(e.authed, "/project", cuerpoProyecto("Juego", proyectoMinimo, noJPEG))
	exigirStatus(t, otro, http.StatusBadRequest)
	if msg := errorJSON(t, otro); msg != "el thumbnail debe ser jpeg" {
		t.Fatalf("base64: %q", msg)
	}

	sinThumb := e.crearProyecto(e.authed, "Juego", proyectoMinimo, "")
	id := strconv.FormatFloat(sinThumb["id"].(float64), 'f', 0, 64)
	thumb := e.hacer(e.anonimo(), http.MethodGet, "/project/"+id+"/thumb", "")
	exigirStatus(t, thumb, http.StatusOK)
	cuerpo := leerCuerpo(t, thumb)
	if len(cuerpo) < 2 || cuerpo[0] != 0xff || cuerpo[1] != 0xd8 {
		t.Fatal("el thumb por defecto no es jpeg")
	}

	vacio := e.postJSON(e.authed, "/project", cuerpoProyecto("   ", proyectoMinimo, ""))
	exigirStatus(t, vacio, http.StatusBadRequest)
	if msg := errorJSON(t, vacio); msg != "el nombre del proyecto no puede estar vacio" {
		t.Fatalf("vacio: %q", msg)
	}

	largo := e.postJSON(e.authed, "/project", cuerpoProyecto(strings.Repeat("á", 81), proyectoMinimo, ""))
	exigirStatus(t, largo, http.StatusBadRequest)
	if msg := errorJSON(t, largo); msg != "el nombre del proyecto es demasiado largo" {
		t.Fatalf("largo: %q", msg)
	}

	arr := e.postJSON(e.authed, "/project", cuerpoProyecto("Juego", `{"Escenas":[]}`, ""))
	exigirStatus(t, arr, http.StatusBadRequest)
	if msg := errorJSON(t, arr); msg != "Escenas debe ser un objeto" {
		t.Fatalf("escenas: %q", msg)
	}

	sonidos := e.postJSON(e.authed, "/project", cuerpoProyecto("Juego", `{"Sonidos":[]}`, ""))
	exigirStatus(t, sonidos, http.StatusBadRequest)
	if msg := errorJSON(t, sonidos); msg != "Sonidos debe ser un objeto" {
		t.Fatalf("sonidos: %q", msg)
	}
	musica := e.postJSON(e.authed, "/project", cuerpoProyecto("Juego", `{"Musica":[]}`, ""))
	exigirStatus(t, musica, http.StatusBadRequest)
	if msg := errorJSON(t, musica); msg != "Musica debe ser un objeto" {
		t.Fatalf("musica: %q", msg)
	}
	audio := e.postJSON(e.authed, "/project", cuerpoProyecto("Juego", `{"Sonidos":{},"Musica":{}}`, ""))
	exigirStatus(t, audio, http.StatusCreated)
	_ = leerCuerpo(t, audio)
}

func TestPermisosDeProyecto(t *testing.T) {
	e := nuevoEntorno(t)
	admin := e.clienteNuevo()
	e.registrar(admin, "admin", "Admin", "clave-123")
	dueno := e.clienteNuevo()
	e.registrar(dueno, "ana", "Ana", "clave-123")
	ajeno := e.clienteNuevo()
	e.registrar(ajeno, "beto", "Beto", "clave-123")

	creado := e.crearProyecto(dueno, "Juego", proyectoMinimo, "")
	id := strconv.FormatFloat(creado["id"].(float64), 'f', 0, 64)
	path := "/project/" + id

	editAjeno := e.postJSON(ajeno, path, cuerpoProyecto("Robado", `{}`, ""))
	exigirStatus(t, editAjeno, http.StatusForbidden)
	if msg := errorJSON(t, editAjeno); msg != "no tenes permiso para editar este proyecto" {
		t.Fatalf("editar: %q", msg)
	}
	delAjeno := e.hacer(ajeno, http.MethodDelete, path, "")
	exigirStatus(t, delAjeno, http.StatusForbidden)
	if msg := errorJSON(t, delAjeno); msg != "no tenes permiso para borrar este proyecto" {
		t.Fatalf("borrar: %q", msg)
	}

	editDueno := e.postJSON(dueno, path, cuerpoProyecto("Nuevo", `{"Escenas":{}}`, ""))
	exigirStatus(t, editDueno, http.StatusOK)
	_ = leerCuerpo(t, editDueno)

	editAdmin := e.postJSON(admin, path, cuerpoProyecto("Por admin", `{"Objetos":{}}`, ""))
	exigirStatus(t, editAdmin, http.StatusOK)
	_ = leerCuerpo(t, editAdmin)
	ver := e.hacer(e.anonimo(), http.MethodGet, "/api/project/"+id, "")
	exigirStatus(t, ver, http.StatusOK)
	var det map[string]any
	decodificarJSON(t, ver, &det)
	if det["nombre"] != "Por admin" {
		t.Fatalf("nombre = %v", det["nombre"])
	}

	delAdmin := e.hacer(admin, http.MethodDelete, path, "")
	exigirStatus(t, delAdmin, http.StatusNoContent)
	_ = leerCuerpo(t, delAdmin)
	despues := e.hacer(e.anonimo(), http.MethodGet, "/api/project/"+id, "")
	exigirStatus(t, despues, http.StatusNotFound)
	_ = leerCuerpo(t, despues)
}

func TestDuenoBorraProyecto(t *testing.T) {
	e := nuevoEntorno(t)
	e.registrar(e.authed, "ana", "Ana", "clave-123")
	creado := e.crearProyecto(e.authed, "Juego", proyectoMinimo, "")
	id := strconv.FormatFloat(creado["id"].(float64), 'f', 0, 64)
	del := e.hacer(e.authed, http.MethodDelete, "/project/"+id, "")
	exigirStatus(t, del, http.StatusNoContent)
	_ = leerCuerpo(t, del)
	ver := e.hacer(e.anonimo(), http.MethodGet, "/api/project/"+id, "")
	exigirStatus(t, ver, http.StatusNotFound)
	if msg := errorJSON(t, ver); msg != "proyecto no encontrado" {
		t.Fatalf("msg = %q", msg)
	}
}

func TestListarProyectos(t *testing.T) {
	e := nuevoEntorno(t)
	ana := e.clienteNuevo()
	e.registrar(ana, "ana", "Ana", "clave-123")
	beto := e.clienteNuevo()
	e.registrar(beto, "beto", "Beto", "clave-123")

	var idsAna []string
	for i := 0; i < 6; i++ {
		creado := e.crearProyecto(ana, "A"+strconv.Itoa(i), `{}`, "")
		idsAna = append(idsAna, strconv.FormatFloat(creado["id"].(float64), 'f', 0, 64))
		time.Sleep(2 * time.Millisecond)
	}
	deBeto := e.crearProyecto(beto, "De Beto", `{}`, "")
	time.Sleep(2 * time.Millisecond)

	resp := e.hacer(e.anonimo(), http.MethodGet, "/api/projects", "")
	exigirStatus(t, resp, http.StatusOK)
	var lista []map[string]any
	decodificarJSON(t, resp, &lista)
	if len(lista) != 5 {
		t.Fatalf("len = %d", len(lista))
	}
	if lista[0]["nombre"] != "De Beto" || lista[0]["ownerNombre"] != "beto" {
		t.Fatalf("primero = %+v", lista[0])
	}
	if lista[0]["thumb"] != "/project/"+strconv.FormatFloat(deBeto["id"].(float64), 'f', 0, 64)+"/thumb" {
		t.Fatalf("thumb = %v", lista[0]["thumb"])
	}
	if lista[4]["nombre"] != "A2" {
		t.Fatalf("ultimo de la pagina = %+v", lista[4])
	}

	deAna := e.hacer(e.anonimo(), http.MethodGet, "/api/user/1/projects", "")
	exigirStatus(t, deAna, http.StatusOK)
	var soloAna []map[string]any
	decodificarJSON(t, deAna, &soloAna)
	if len(soloAna) != 6 {
		t.Fatalf("ana = %d", len(soloAna))
	}
	if soloAna[0]["nombre"] != "A5" || soloAna[0]["ownerNombre"] != "ana" {
		t.Fatalf("primero de ana = %+v", soloAna[0])
	}
	for _, p := range soloAna {
		if p["ownerNombre"] != "ana" {
			t.Fatalf("mezclado: %+v", p)
		}
	}
	if idsAna[0] == "" {
		t.Fatal("ids vacios")
	}

	no := e.hacer(e.anonimo(), http.MethodGet, "/api/user/9/projects", "")
	exigirStatus(t, no, http.StatusNotFound)
	if msg := errorJSON(t, no); msg != "usuario no encontrado" {
		t.Fatalf("msg = %q", msg)
	}
}

func TestPaginaDeProyecto(t *testing.T) {
	e := nuevoEntorno(t)
	e.registrar(e.authed, "ana", "Ana", "clave-123")
	e.crearProyecto(e.authed, "Juego", proyectoMinimo, "")

	noNum := e.hacer(e.anonimo(), http.MethodGet, "/project/abc", "")
	exigirStatus(t, noNum, http.StatusNotFound)
	if cuerpo := string(leerCuerpo(t, noNum)); !strings.Contains(cuerpo, "SITIO_404") {
		t.Fatalf("cuerpo = %q", cuerpo)
	}
	falta := e.hacer(e.anonimo(), http.MethodGet, "/project/9", "")
	exigirStatus(t, falta, http.StatusNotFound)
	if cuerpo := string(leerCuerpo(t, falta)); !strings.Contains(cuerpo, "SITIO_404") {
		t.Fatalf("cuerpo = %q", cuerpo)
	}
	ok := e.hacer(e.anonimo(), http.MethodGet, "/project/1", "")
	exigirStatus(t, ok, http.StatusOK)
	if cuerpo := string(leerCuerpo(t, ok)); cuerpo != "EDITOR" {
		t.Fatalf("cuerpo = %q", cuerpo)
	}
}
