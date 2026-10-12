package db

import (
	"errors"
	"regexp"
	"testing"
	"time"
)

func TestCrearYLeerSesion(t *testing.T) {
	d := abrir(t)
	u, err := d.CrearUsuario("ana", "Ana", "clave-123", false)
	if err != nil {
		t.Fatal(err)
	}
	token, err := d.CrearSesion(u.ID, time.Hour)
	if err != nil {
		t.Fatal(err)
	}
	if !regexp.MustCompile(`^[0-9a-f]{64}$`).MatchString(token) {
		t.Fatalf("token = %q", token)
	}
	s, err := d.Sesion(token)
	if err != nil {
		t.Fatal(err)
	}
	if s.UserID != u.ID {
		t.Fatalf("userID = %d", s.UserID)
	}
}

func TestSesionDesconocida(t *testing.T) {
	d := abrir(t)
	if _, err := d.Sesion(""); !errors.Is(err, ErrNoEncontrado) {
		t.Fatalf("vacio: %v", err)
	}
	if _, err := d.Sesion("no-existe"); !errors.Is(err, ErrNoEncontrado) {
		t.Fatalf("desconocido: %v", err)
	}
}

func TestSesionExpiradaSeBorra(t *testing.T) {
	d := abrir(t)
	u, err := d.CrearUsuario("ana", "Ana", "clave-123", false)
	if err != nil {
		t.Fatal(err)
	}
	for _, ttl := range []time.Duration{0, -time.Second} {
		token, err := d.CrearSesion(u.ID, ttl)
		if err != nil {
			t.Fatal(err)
		}
		if _, err := d.Sesion(token); !errors.Is(err, ErrExpirado) {
			t.Fatalf("ttl %s: %v", ttl, err)
		}
		if _, err := d.Sesion(token); !errors.Is(err, ErrNoEncontrado) {
			t.Fatalf("segunda lectura ttl %s: %v", ttl, err)
		}
	}
}

func TestBorrarSesion(t *testing.T) {
	d := abrir(t)
	u, err := d.CrearUsuario("ana", "Ana", "clave-123", false)
	if err != nil {
		t.Fatal(err)
	}
	token, err := d.CrearSesion(u.ID, time.Hour)
	if err != nil {
		t.Fatal(err)
	}
	if err := d.BorrarSesion(token); err != nil {
		t.Fatal(err)
	}
	if _, err := d.Sesion(token); !errors.Is(err, ErrNoEncontrado) {
		t.Fatalf("despues de borrar: %v", err)
	}
	if err := d.BorrarSesion(""); err != nil {
		t.Fatal(err)
	}
}
