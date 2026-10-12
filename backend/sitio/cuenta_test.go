package sitio

import (
	"net/http"
	"strings"
	"testing"
)

func TestCuentaSinSesion(t *testing.T) {
	e := nuevoEntorno(t)
	resp := e.postJSON(e.anonimo(), "/api/account", map[string]string{
		"passwordActual": "clave-123",
		"display":        "Otra",
	})
	exigirStatus(t, resp, http.StatusUnauthorized)
	if msg := errorJSON(t, resp); msg != "no autenticado" {
		t.Fatalf("msg = %q", msg)
	}
}

func TestCuentaClaveActualMala(t *testing.T) {
	e := nuevoEntorno(t)
	e.registrar(e.authed, "ana", "Ana", "clave-123")
	resp := e.postJSON(e.authed, "/api/account", map[string]string{
		"passwordActual": "no-es",
		"display":        "Otra",
		"passwordNueva":  "clave-nueva",
	})
	exigirStatus(t, resp, http.StatusUnauthorized)
	if msg := errorJSON(t, resp); msg != "contraseña actual incorrecta" {
		t.Fatalf("msg = %q", msg)
	}
	u, err := e.base.UsuarioPorNombre("ana")
	if err != nil {
		t.Fatal(err)
	}
	if u.Display != "Ana" || !e.base.VerificarPassword(u, "clave-123") {
		t.Fatalf("la cuenta cambio: %+v", u.Publico())
	}
}

func TestCuentaCambiaDisplay(t *testing.T) {
	e := nuevoEntorno(t)
	creado := e.registrar(e.authed, "ana", "Ana", "clave-123")
	resp := e.postJSON(e.authed, "/api/account", map[string]string{
		"passwordActual": "clave-123",
		"display":        "Ana Nueva",
	})
	exigirStatus(t, resp, http.StatusOK)
	var out map[string]any
	decodificarJSON(t, resp, &out)
	if out["display"] != "Ana Nueva" {
		t.Fatalf("display = %v", out["display"])
	}
	id := creado["id"].(float64)
	ver := e.hacer(e.anonimo(), http.MethodGet, "/api/user/1", "")
	if id != 1 {
		t.Fatalf("id = %v", creado["id"])
	}
	exigirStatus(t, ver, http.StatusOK)
	var pub map[string]any
	decodificarJSON(t, ver, &pub)
	if pub["display"] != "Ana Nueva" {
		t.Fatalf("publico = %+v", pub)
	}
}

func TestCuentaDisplayLargo(t *testing.T) {
	e := nuevoEntorno(t)
	e.registrar(e.authed, "ana", "Ana", "clave-123")
	resp := e.postJSON(e.authed, "/api/account", map[string]string{
		"passwordActual": "clave-123",
		"display":        strings.Repeat("á", 41),
	})
	exigirStatus(t, resp, http.StatusBadRequest)
	if msg := errorJSON(t, resp); msg != "el nombre visible es demasiado largo" {
		t.Fatalf("msg = %q", msg)
	}
}

func TestCuentaCambiaPassword(t *testing.T) {
	e := nuevoEntorno(t)
	e.registrar(e.authed, "ana", "Ana", "clave-123")
	resp := e.postJSON(e.authed, "/api/account", map[string]string{
		"passwordActual": "clave-123",
		"passwordNueva":  "clave-nueva",
	})
	exigirStatus(t, resp, http.StatusOK)
	_ = leerCuerpo(t, resp)

	viejo := e.postJSON(e.clienteNuevo(), "/api/login", map[string]string{
		"nombre":   "ana",
		"password": "clave-123",
	})
	exigirStatus(t, viejo, http.StatusUnauthorized)
	_ = leerCuerpo(t, viejo)

	nuevo := e.postJSON(e.clienteNuevo(), "/api/login", map[string]string{
		"nombre":   "ana",
		"password": "clave-nueva",
	})
	exigirStatus(t, nuevo, http.StatusOK)
	_ = leerCuerpo(t, nuevo)
}

func TestGetUserErrores(t *testing.T) {
	e := nuevoEntorno(t)
	no := e.hacer(e.anonimo(), http.MethodGet, "/api/user/4", "")
	exigirStatus(t, no, http.StatusNotFound)
	if msg := errorJSON(t, no); msg != "usuario no encontrado" {
		t.Fatalf("msg = %q", msg)
	}
	cero := e.hacer(e.anonimo(), http.MethodGet, "/api/user/0", "")
	exigirStatus(t, cero, http.StatusBadRequest)
	if msg := errorJSON(t, cero); msg != "id invalido" {
		t.Fatalf("msg = %q", msg)
	}
	letra := e.hacer(e.anonimo(), http.MethodGet, "/api/user/abc", "")
	exigirStatus(t, letra, http.StatusBadRequest)
	if msg := errorJSON(t, letra); msg != "id invalido" {
		t.Fatalf("msg = %q", msg)
	}
}
