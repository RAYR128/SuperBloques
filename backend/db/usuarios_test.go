package db

import (
	"encoding/json"
	"errors"
	"testing"
)

func TestCrearUsuarioPrimeroEsAdmin(t *testing.T) {
	d := abrir(t)
	primero, err := d.CrearUsuario("ana", "Ana", "clave-123", false)
	if err != nil {
		t.Fatal(err)
	}
	if !primero.Admin || primero.ID != 1 {
		t.Fatalf("primero = %+v", primero)
	}
	segundo, err := d.CrearUsuario("beto", "Beto", "clave-123", false)
	if err != nil {
		t.Fatal(err)
	}
	if segundo.Admin || segundo.ID != 2 {
		t.Fatalf("segundo = %+v", segundo)
	}
}

func TestCrearUsuarioForzarAdmin(t *testing.T) {
	d := abrir(t)
	if _, err := d.CrearUsuario("ana", "Ana", "clave-123", false); err != nil {
		t.Fatal(err)
	}
	admin, err := d.CrearUsuario("root", "Root", "clave-123", true)
	if err != nil {
		t.Fatal(err)
	}
	if !admin.Admin {
		t.Fatal("forzarAdmin no marco admin")
	}
}

func TestCrearUsuarioNombreDuplicadoSinCaso(t *testing.T) {
	d := abrir(t)
	if _, err := d.CrearUsuario("Ana", "Ana", "clave-123", false); err != nil {
		t.Fatal(err)
	}
	_, err := d.CrearUsuario("ana", "Otra", "clave-123", false)
	if !errors.Is(err, ErrDuplicado) {
		t.Fatalf("err = %v", err)
	}
}

func TestUsuarioNoEncontrado(t *testing.T) {
	d := abrir(t)
	if _, err := d.Usuario(9); !errors.Is(err, ErrNoEncontrado) {
		t.Fatalf("Usuario: %v", err)
	}
	if _, err := d.UsuarioPorNombre("nadie"); !errors.Is(err, ErrNoEncontrado) {
		t.Fatalf("UsuarioPorNombre: %v", err)
	}
}

func TestVerificarYCambiarPassword(t *testing.T) {
	d := abrir(t)
	u, err := d.CrearUsuario("ana", "Ana", "clave-123", false)
	if err != nil {
		t.Fatal(err)
	}
	if !d.VerificarPassword(u, "clave-123") {
		t.Fatal("la clave correcta fue rechazada")
	}
	if d.VerificarPassword(u, "otra-clave") {
		t.Fatal("una clave mala fue aceptada")
	}
	if d.VerificarPassword(nil, "clave-123") {
		t.Fatal("nil fue aceptado")
	}
	if err := d.CambiarPassword(u, "clave-nueva"); err != nil {
		t.Fatal(err)
	}
	if d.VerificarPassword(u, "clave-123") {
		t.Fatal("la clave vieja sigue valida")
	}
	if !d.VerificarPassword(u, "clave-nueva") {
		t.Fatal("la clave nueva no valida")
	}
}

func TestBootstrapAdmin(t *testing.T) {
	d := abrir(t)
	if err := d.BootstrapAdmin("root", "clave-123"); err != nil {
		t.Fatal(err)
	}
	creado, err := d.UsuarioPorNombre("root")
	if err != nil {
		t.Fatal(err)
	}
	if !creado.Admin || creado.ID != 1 {
		t.Fatalf("creado = %+v", creado)
	}

	segundo, err := d.CrearUsuario("ana", "Ana", "clave-123", false)
	if err != nil {
		t.Fatal(err)
	}
	if segundo.Admin {
		t.Fatal("el segundo usuario no deberia ser admin")
	}
	if err := d.BootstrapAdmin("ana", "clave-nueva"); err != nil {
		t.Fatal(err)
	}
	actualizado, err := d.Usuario(segundo.ID)
	if err != nil {
		t.Fatal(err)
	}
	if actualizado.ID != segundo.ID || !actualizado.Admin {
		t.Fatalf("actualizado = %+v", actualizado)
	}
	if d.VerificarPassword(actualizado, "clave-123") {
		t.Fatal("el hash viejo sigue")
	}
	if !d.VerificarPassword(actualizado, "clave-nueva") {
		t.Fatal("el hash nuevo no valida")
	}
}

func TestPublicoNoIncluyeHash(t *testing.T) {
	d := abrir(t)
	u, err := d.CrearUsuario("ana", "Ana", "clave-123", false)
	if err != nil {
		t.Fatal(err)
	}
	raw, err := json.Marshal(u.Publico())
	if err != nil {
		t.Fatal(err)
	}
	var m map[string]any
	if err := json.Unmarshal(raw, &m); err != nil {
		t.Fatal(err)
	}
	if _, ok := m["hash"]; ok {
		t.Fatalf("publico incluye hash: %s", raw)
	}
	if m["nombre"] != "ana" || m["display"] != "Ana" {
		t.Fatalf("publico = %s", raw)
	}
}
