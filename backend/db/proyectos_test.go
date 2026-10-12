package db

import (
	"bytes"
	"errors"
	"testing"
	"time"
)

func TestCrearProyecto(t *testing.T) {
	d := abrir(t)
	jsonProyecto := []byte(`{"Escenas":{},"Objetos":{}}`)
	p, err := d.CrearProyecto(1, "Uno", jsonProyecto, nil)
	if err != nil {
		t.Fatal(err)
	}
	if p.ID != 1 || p.OwnerID != 1 || p.Nombre != "Uno" {
		t.Fatalf("meta = %+v", p)
	}
	datos, err := d.ProyectoDatos(p.ID)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(datos, jsonProyecto) {
		t.Fatalf("datos = %s", datos)
	}
	if _, err := d.Thumb(p.ID); !errors.Is(err, ErrNoEncontrado) {
		t.Fatalf("thumb vacio: %v", err)
	}

	thumb := []byte{0xff, 0xd8, 0xff, 0x01}
	conThumb, err := d.CrearProyecto(1, "Dos", jsonProyecto, thumb)
	if err != nil {
		t.Fatal(err)
	}
	if conThumb.ID != 2 {
		t.Fatalf("id = %d", conThumb.ID)
	}
	got, err := d.Thumb(conThumb.ID)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(got, thumb) {
		t.Fatalf("thumb = %v", got)
	}
	delDueno, err := d.ProyectosDeUsuario(1)
	if err != nil {
		t.Fatal(err)
	}
	if len(delDueno) != 2 {
		t.Fatalf("proyectos del dueno = %d", len(delDueno))
	}
}

func TestActualizarProyecto(t *testing.T) {
	d := abrir(t)
	viejo := []byte(`{"Escenas":{}}`)
	thumb := []byte{0xff, 0xd8, 0xff, 0x01}
	primero, err := d.CrearProyecto(1, "Viejo", viejo, thumb)
	if err != nil {
		t.Fatal(err)
	}
	time.Sleep(2 * time.Millisecond)
	segundo, err := d.CrearProyecto(1, "Otro", viejo, nil)
	if err != nil {
		t.Fatal(err)
	}
	time.Sleep(2 * time.Millisecond)
	nombre := "Nuevo"
	nuevo := []byte(`{"Objetos":{}}`)
	act, err := d.ActualizarProyecto(primero.ID, &nombre, nuevo, nil)
	if err != nil {
		t.Fatal(err)
	}
	if act.Nombre != "Nuevo" {
		t.Fatalf("nombre = %s", act.Nombre)
	}
	datos, err := d.ProyectoDatos(primero.ID)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(datos, nuevo) {
		t.Fatalf("datos = %s", datos)
	}
	gotThumb, err := d.Thumb(primero.ID)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(gotThumb, thumb) {
		t.Fatal("el thumb se borro en una actualizacion sin thumb")
	}
	recientes, err := d.ProyectosRecientes(10)
	if err != nil {
		t.Fatal(err)
	}
	if len(recientes) != 2 {
		t.Fatalf("recientes = %d", len(recientes))
	}
	if recientes[0].ID != primero.ID || recientes[1].ID != segundo.ID {
		t.Fatalf("orden = %d, %d", recientes[0].ID, recientes[1].ID)
	}
}

func TestBorrarProyecto(t *testing.T) {
	d := abrir(t)
	p, err := d.CrearProyecto(1, "Uno", []byte(`{}`), []byte{0xff, 0xd8, 0xff})
	if err != nil {
		t.Fatal(err)
	}
	if err := d.BorrarProyecto(p.ID); err != nil {
		t.Fatal(err)
	}
	if _, err := d.Proyecto(p.ID); !errors.Is(err, ErrNoEncontrado) {
		t.Fatalf("meta: %v", err)
	}
	if _, err := d.ProyectoDatos(p.ID); !errors.Is(err, ErrNoEncontrado) {
		t.Fatalf("datos: %v", err)
	}
	if _, err := d.Thumb(p.ID); !errors.Is(err, ErrNoEncontrado) {
		t.Fatalf("thumb: %v", err)
	}
	delDueno, err := d.ProyectosDeUsuario(1)
	if err != nil {
		t.Fatal(err)
	}
	if len(delDueno) != 0 {
		t.Fatalf("quedaron %d proyectos del dueno", len(delDueno))
	}
	recientes, err := d.ProyectosRecientes(10)
	if err != nil {
		t.Fatal(err)
	}
	if len(recientes) != 0 {
		t.Fatalf("quedaron %d recientes", len(recientes))
	}
	if err := d.BorrarProyecto(p.ID); !errors.Is(err, ErrNoEncontrado) {
		t.Fatalf("borrar inexistente: %v", err)
	}
}

func TestProyectosRecientes(t *testing.T) {
	d := abrir(t)
	var ids []uint64
	for i := 0; i < 6; i++ {
		p, err := d.CrearProyecto(1, "P", []byte(`{}`), nil)
		if err != nil {
			t.Fatal(err)
		}
		ids = append(ids, p.ID)
		time.Sleep(2 * time.Millisecond)
	}
	if got := ids[0]; got != 1 {
		t.Fatalf("primer id = %d", got)
	}
	recientes, err := d.ProyectosRecientes(5)
	if err != nil {
		t.Fatal(err)
	}
	if len(recientes) != 5 {
		t.Fatalf("len = %d", len(recientes))
	}
	want := []uint64{ids[5], ids[4], ids[3], ids[2], ids[1]}
	for i, id := range want {
		if recientes[i].ID != id {
			t.Fatalf("pos %d = %d, want %d", i, recientes[i].ID, id)
		}
	}
	if lista, err := d.ProyectosRecientes(0); err != nil || lista != nil {
		t.Fatalf("n=0: %v %v", lista, err)
	}
}

func TestProyectosDeUsuarioOrdenados(t *testing.T) {
	d := abrir(t)
	a1, err := d.CrearProyecto(1, "A1", []byte(`{}`), nil)
	if err != nil {
		t.Fatal(err)
	}
	time.Sleep(2 * time.Millisecond)
	if _, err := d.CrearProyecto(2, "B", []byte(`{}`), nil); err != nil {
		t.Fatal(err)
	}
	time.Sleep(2 * time.Millisecond)
	a2, err := d.CrearProyecto(1, "A2", []byte(`{}`), nil)
	if err != nil {
		t.Fatal(err)
	}
	lista, err := d.ProyectosDeUsuario(1)
	if err != nil {
		t.Fatal(err)
	}
	if len(lista) != 2 || lista[0].ID != a2.ID || lista[1].ID != a1.ID {
		t.Fatalf("lista = %+v", lista)
	}
	otro, err := d.ProyectosDeUsuario(2)
	if err != nil {
		t.Fatal(err)
	}
	if len(otro) != 1 || otro[0].Nombre != "B" {
		t.Fatalf("otro = %+v", otro)
	}
}
