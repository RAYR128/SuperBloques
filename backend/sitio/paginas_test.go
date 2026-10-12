package sitio

import (
	"net/http"
	"testing"
)

func TestPaginasAnonimasYHome(t *testing.T) {
	e := nuevoEntorno(t)
	login := e.hacer(e.anonimo(), http.MethodGet, "/login", "")
	exigirStatus(t, login, http.StatusOK)
	if string(leerCuerpo(t, login)) != "SITIO_LOGIN" {
		t.Fatal("login no sirvio su html")
	}
	reg := e.hacer(e.anonimo(), http.MethodGet, "/register", "")
	exigirStatus(t, reg, http.StatusOK)
	if string(leerCuerpo(t, reg)) != "SITIO_REGISTER" {
		t.Fatal("register no sirvio su html")
	}
	home := e.hacer(e.anonimo(), http.MethodGet, "/", "")
	exigirStatus(t, home, http.StatusOK)
	if string(leerCuerpo(t, home)) != "SITIO_INDEX" {
		t.Fatal("home no sirvio su html")
	}
	user := e.hacer(e.anonimo(), http.MethodGet, "/u/1", "")
	exigirStatus(t, user, http.StatusOK)
	if string(leerCuerpo(t, user)) != "SITIO_USER" {
		t.Fatal("perfil no sirvio su html")
	}
}

func TestPaginasConSesionRedirigen(t *testing.T) {
	e := nuevoEntorno(t)
	e.registrar(e.authed, "ana", "Ana", "clave-123")
	c := e.sinSeguir()

	login := e.hacer(c, http.MethodGet, "/login", "")
	exigirStatus(t, login, http.StatusSeeOther)
	if loc := login.Header.Get("Location"); loc != "/" {
		t.Fatalf("login location = %q", loc)
	}
	_ = leerCuerpo(t, login)

	next := e.hacer(c, http.MethodGet, "/login?next=/account", "")
	exigirStatus(t, next, http.StatusSeeOther)
	if loc := next.Header.Get("Location"); loc != "/account" {
		t.Fatalf("next location = %q", loc)
	}
	_ = leerCuerpo(t, next)

	evil := e.hacer(c, http.MethodGet, "/login?next=//evil", "")
	exigirStatus(t, evil, http.StatusSeeOther)
	if loc := evil.Header.Get("Location"); loc != "/" {
		t.Fatalf("//evil location = %q", loc)
	}
	_ = leerCuerpo(t, evil)

	abs := e.hacer(c, http.MethodGet, "/login?next=https://evil", "")
	exigirStatus(t, abs, http.StatusSeeOther)
	if loc := abs.Header.Get("Location"); loc != "/" {
		t.Fatalf("abs location = %q", loc)
	}
	_ = leerCuerpo(t, abs)

	reg := e.hacer(c, http.MethodGet, "/register?next=/u/1", "")
	exigirStatus(t, reg, http.StatusSeeOther)
	if loc := reg.Header.Get("Location"); loc != "/u/1" {
		t.Fatalf("register location = %q", loc)
	}
	_ = leerCuerpo(t, reg)
}

func TestPaginaCuenta(t *testing.T) {
	e := nuevoEntorno(t)
	anon := e.hacer(e.anonimo(), http.MethodGet, "/account", "")
	exigirStatus(t, anon, http.StatusSeeOther)
	if loc := anon.Header.Get("Location"); loc != "/login?next=/account" {
		t.Fatalf("location = %q", loc)
	}
	_ = leerCuerpo(t, anon)

	e.registrar(e.authed, "ana", "Ana", "clave-123")
	ok := e.hacer(e.sinSeguir(), http.MethodGet, "/account", "")
	exigirStatus(t, ok, http.StatusOK)
	if string(leerCuerpo(t, ok)) != "SITIO_ACCOUNT" {
		t.Fatal("account no sirvio su html")
	}
}
