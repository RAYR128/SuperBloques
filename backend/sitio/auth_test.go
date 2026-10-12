package sitio

import (
	"net/http"
	"strings"
	"testing"
)

func TestRegisterLoginYMe(t *testing.T) {
	e := nuevoEntorno(t)
	resp := e.postJSON(e.authed, "/api/register", map[string]string{
		"nombre":   "ana",
		"display":  "Ana",
		"password": "clave-123",
	})
	exigirStatus(t, resp, http.StatusCreated)
	var publico map[string]any
	decodificarJSON(t, resp, &publico)
	if _, ok := publico["hash"]; ok {
		t.Fatalf("la respuesta incluye hash: %+v", publico)
	}
	if publico["nombre"] != "ana" || publico["display"] != "Ana" || publico["admin"] != true {
		t.Fatalf("publico = %+v", publico)
	}
	c := cookieSesion(t, resp)
	if c.Value == "" || !c.HttpOnly || c.SameSite != http.SameSiteLaxMode || c.Path != "/" || c.Secure {
		t.Fatalf("cookie = %+v", c)
	}

	me := e.hacer(e.authed, http.MethodGet, "/api/me", "")
	exigirStatus(t, me, http.StatusOK)
	var yo map[string]any
	decodificarJSON(t, me, &yo)
	if yo["nombre"] != "ana" {
		t.Fatalf("me = %+v", yo)
	}

	otro := e.clienteNuevo()
	segundo := e.registrar(otro, "beto", "Beto", "clave-123")
	if segundo["admin"] != false {
		t.Fatalf("segundo = %+v", segundo)
	}

	dup := e.postJSON(e.clienteNuevo(), "/api/register", map[string]string{
		"nombre":   "ANA",
		"password": "clave-123",
	})
	exigirStatus(t, dup, http.StatusConflict)
	if msg := errorJSON(t, dup); msg != "ese usuario ya existe" {
		t.Fatalf("msg = %q", msg)
	}

	malo := e.postJSON(e.clienteNuevo(), "/api/login", map[string]string{
		"nombre":   "ana",
		"password": "no-es",
	})
	exigirStatus(t, malo, http.StatusUnauthorized)
	if msg := errorJSON(t, malo); msg != "usuario o contraseña incorrectos" {
		t.Fatalf("msg = %q", msg)
	}
	inexistente := e.postJSON(e.clienteNuevo(), "/api/login", map[string]string{
		"nombre":   "nadie",
		"password": "clave-123",
	})
	exigirStatus(t, inexistente, http.StatusUnauthorized)
	if msg := errorJSON(t, inexistente); msg != "usuario o contraseña incorrectos" {
		t.Fatalf("msg = %q", msg)
	}

	ok := e.postJSON(otro, "/api/login", map[string]string{
		"nombre":   "beto",
		"password": "clave-123",
	})
	exigirStatus(t, ok, http.StatusOK)
	cookieSesion(t, ok)
	_ = leerCuerpo(t, ok)

	sinCookie := e.hacer(e.anonimo(), http.MethodGet, "/api/me", "")
	exigirStatus(t, sinCookie, http.StatusUnauthorized)
	_ = leerCuerpo(t, sinCookie)
}

func TestRegisterValidacion(t *testing.T) {
	e := nuevoEntorno(t)
	casos := []struct {
		nombre   string
		display  string
		password string
		msg      string
	}{
		{"ab", "Ana", "clave-123", "el usuario debe tener 3-24 caracteres (letras, numeros o _)"},
		{"ana", "Ana", "corta", "la contraseña debe tener al menos 8 caracteres"},
		{"ana", "Ana", strings.Repeat("a", 73), "la contraseña es demasiado larga"},
		{"ana", "ho\nla", "clave-123", "el nombre visible es invalido"},
	}
	for _, c := range casos {
		resp := e.postJSON(e.clienteNuevo(), "/api/register", map[string]string{
			"nombre":   c.nombre,
			"display":  c.display,
			"password": c.password,
		})
		exigirStatus(t, resp, http.StatusBadRequest)
		if msg := errorJSON(t, resp); msg != c.msg {
			t.Fatalf("nombre %q: msg = %q", c.nombre, msg)
		}
	}

	roto := e.postJSON(e.clienteNuevo(), "/api/register", "{")
	exigirStatus(t, roto, http.StatusBadRequest)
	if msg := errorJSON(t, roto); msg != "json invalido" {
		t.Fatalf("msg = %q", msg)
	}

	vacio := e.registrar(e.clienteNuevo(), "ana", "", "clave-123")
	if vacio["display"] != "ana" {
		t.Fatalf("display = %v", vacio["display"])
	}
}

func TestLogout(t *testing.T) {
	e := nuevoEntorno(t)
	e.registrar(e.authed, "ana", "Ana", "clave-123")
	me := e.hacer(e.authed, http.MethodGet, "/api/me", "")
	exigirStatus(t, me, http.StatusOK)
	_ = leerCuerpo(t, me)

	out := e.hacer(e.authed, http.MethodPost, "/api/logout", "")
	exigirStatus(t, out, http.StatusNoContent)
	c := cookieSesion(t, out)
	if c.MaxAge >= 0 || c.Value != "" {
		t.Fatalf("cookie de logout = %+v", c)
	}
	_ = leerCuerpo(t, out)

	otra := e.hacer(e.authed, http.MethodGet, "/api/me", "")
	exigirStatus(t, otra, http.StatusUnauthorized)
	_ = leerCuerpo(t, otra)

	sin := e.hacer(e.anonimo(), http.MethodPost, "/api/logout", "")
	exigirStatus(t, sin, http.StatusUnauthorized)
	_ = leerCuerpo(t, sin)
}

func TestHealth(t *testing.T) {
	e := nuevoEntorno(t)
	resp := e.hacer(e.anonimo(), http.MethodGet, "/api/health", "")
	exigirStatus(t, resp, http.StatusOK)
	var m map[string]string
	decodificarJSON(t, resp, &m)
	if m["status"] != "ok" {
		t.Fatalf("health = %+v", m)
	}
}

func TestCookieSecureConTrustHTTPS(t *testing.T) {
	t.Setenv("TRUST_HTTPS", "1")
	e := abrirEntorno(t)
	resp := e.postJSON(e.authed, "/api/register", map[string]string{
		"nombre":   "ana",
		"password": "clave-123",
	})
	exigirStatus(t, resp, http.StatusCreated)
	c := cookieSesion(t, resp)
	if !c.Secure {
		t.Fatal("la cookie no salio Secure")
	}
	_ = leerCuerpo(t, resp)
}
