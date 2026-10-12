package sitio

import (
	"bytes"
	"encoding/json"
	"io"
	"net/http"
	"net/http/cookiejar"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"superbloques/db"

	"golang.org/x/crypto/bcrypt"
)

func TestMain(m *testing.M) {
	db.CostoBcrypt = bcrypt.MinCost
	os.Exit(m.Run())
}

type entorno struct {
	t      *testing.T
	srv    *httptest.Server
	base   *db.DB
	authed *http.Client
}

func nuevoEntorno(t *testing.T) *entorno {
	t.Helper()
	t.Setenv("TRUST_HTTPS", "")
	return abrirEntorno(t)
}

func abrirEntorno(t *testing.T) *entorno {
	t.Helper()
	base, err := db.Open(filepath.Join(t.TempDir(), "t.db"))
	if err != nil {
		t.Fatalf("abrir db: %v", err)
	}
	sitioDir := t.TempDir()
	editorDir := t.TempDir()
	paginas := map[string]string{
		"index.html":    "SITIO_INDEX",
		"login.html":    "SITIO_LOGIN",
		"register.html": "SITIO_REGISTER",
		"account.html":  "SITIO_ACCOUNT",
		"user.html":     "SITIO_USER",
		"404.html":      "SITIO_404",
	}
	for nombre, cuerpo := range paginas {
		if err := os.WriteFile(filepath.Join(sitioDir, nombre), []byte(cuerpo), 0o644); err != nil {
			t.Fatal(err)
		}
	}
	if err := os.WriteFile(filepath.Join(editorDir, "index.html"), []byte("EDITOR"), 0o644); err != nil {
		t.Fatal(err)
	}
	h := NewWith(base, editorDir, sitioDir)
	srv := httptest.NewServer(h)
	t.Cleanup(srv.Close)
	t.Cleanup(func() { _ = base.Close() })
	jar, err := cookiejar.New(nil)
	if err != nil {
		t.Fatal(err)
	}
	return &entorno{
		t:      t,
		srv:    srv,
		base:   base,
		authed: &http.Client{Jar: jar},
	}
}

func (e *entorno) anonimo() *http.Client {
	return &http.Client{CheckRedirect: noSeguir}
}

func (e *entorno) sinSeguir() *http.Client {
	return &http.Client{Jar: e.authed.Jar, CheckRedirect: noSeguir}
}

func noSeguir(req *http.Request, via []*http.Request) error {
	return http.ErrUseLastResponse
}

func (e *entorno) url(path string) string {
	return e.srv.URL + path
}

func (e *entorno) hacer(c *http.Client, method, path, cuerpo string) *http.Response {
	e.t.Helper()
	var body io.Reader
	if cuerpo != "" {
		body = strings.NewReader(cuerpo)
	}
	req, err := http.NewRequest(method, e.url(path), body)
	if err != nil {
		e.t.Fatal(err)
	}
	if cuerpo != "" {
		req.Header.Set("Content-Type", "application/json")
	}
	resp, err := c.Do(req)
	if err != nil {
		e.t.Fatal(err)
	}
	return resp
}

func (e *entorno) postJSON(c *http.Client, path string, v any) *http.Response {
	e.t.Helper()
	var buf bytes.Buffer
	switch raw := v.(type) {
	case string:
		buf.WriteString(raw)
	default:
		if err := json.NewEncoder(&buf).Encode(v); err != nil {
			e.t.Fatal(err)
		}
	}
	return e.hacer(c, http.MethodPost, path, buf.String())
}

func leerCuerpo(t *testing.T, resp *http.Response) []byte {
	t.Helper()
	defer resp.Body.Close()
	b, err := io.ReadAll(resp.Body)
	if err != nil {
		t.Fatal(err)
	}
	return b
}

func decodificarJSON(t *testing.T, resp *http.Response, v any) {
	t.Helper()
	if ct := resp.Header.Get("Content-Type"); !strings.Contains(ct, "application/json") {
		t.Fatalf("content-type = %q", ct)
	}
	b := leerCuerpo(t, resp)
	if err := json.Unmarshal(b, v); err != nil {
		t.Fatalf("json %s: %v", b, err)
	}
}

func errorJSON(t *testing.T, resp *http.Response) string {
	t.Helper()
	var m map[string]string
	decodificarJSON(t, resp, &m)
	msg, ok := m["error"]
	if !ok || msg == "" {
		t.Fatalf("sin error: %+v", m)
	}
	return msg
}

func exigirStatus(t *testing.T, resp *http.Response, status int) {
	t.Helper()
	if resp.StatusCode != status {
		b := leerCuerpo(t, resp)
		t.Fatalf("status = %d, want %d, cuerpo = %s", resp.StatusCode, status, b)
	}
}

func cookieSesion(t *testing.T, resp *http.Response) *http.Cookie {
	t.Helper()
	for _, c := range resp.Cookies() {
		if c.Name == cookieName {
			return c
		}
	}
	t.Fatal("falta cookie sb_session")
	return nil
}

func (e *entorno) registrar(c *http.Client, nombre, display, password string) map[string]any {
	e.t.Helper()
	body := map[string]string{"nombre": nombre, "password": password}
	if display != "" {
		body["display"] = display
	}
	resp := e.postJSON(c, "/api/register", body)
	exigirStatus(e.t, resp, http.StatusCreated)
	var out map[string]any
	decodificarJSON(e.t, resp, &out)
	return out
}

func (e *entorno) clienteNuevo() *http.Client {
	e.t.Helper()
	jar, err := cookiejar.New(nil)
	if err != nil {
		e.t.Fatal(err)
	}
	return &http.Client{Jar: jar}
}

const proyectoMinimo = `{"Escenas":{},"Objetos":{}}`
