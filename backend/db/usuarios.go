package db

import (
	"encoding/json"
	"strings"
	"time"

	bolt "go.etcd.io/bbolt"
)

type Usuario struct {
	ID      uint64    `json:"id"`
	Nombre  string    `json:"nombre"`
	Display string    `json:"display"`
	Hash    []byte    `json:"hash"`
	Admin   bool      `json:"admin"`
	Creado  time.Time `json:"creado"`
}

type UsuarioPublico struct {
	ID      uint64    `json:"id"`
	Nombre  string    `json:"nombre"`
	Display string    `json:"display"`
	Admin   bool      `json:"admin"`
	Creado  time.Time `json:"creado"`
}

func (u Usuario) Publico() UsuarioPublico {
	return UsuarioPublico{
		ID:      u.ID,
		Nombre:  u.Nombre,
		Display: u.Display,
		Admin:   u.Admin,
		Creado:  u.Creado,
	}
}

func usernameKey(nombre string) []byte {
	return []byte(strings.ToLower(nombre))
}

func (d *DB) CrearUsuario(nombre, display, password string, forzarAdmin bool) (*Usuario, error) {
	hash, err := hashPassword(password)
	if err != nil {
		return nil, err
	}
	var u Usuario
	err = d.bolt.Update(func(tx *bolt.Tx) error {
		users := tx.Bucket([]byte(bucketUsers))
		names := tx.Bucket([]byte(bucketUsernames))
		if names.Get(usernameKey(nombre)) != nil {
			return ErrDuplicado
		}
		id, err := nextID(tx, keyNextUserID)
		if err != nil {
			return err
		}
		admin := forzarAdmin
		if !admin {
			k, _ := users.Cursor().First()
			admin = k == nil
		}
		u = Usuario{
			ID:      id,
			Nombre:  nombre,
			Display: display,
			Hash:    hash,
			Admin:   admin,
			Creado:  time.Now().UTC(),
		}
		raw, err := json.Marshal(u)
		if err != nil {
			return err
		}
		if err := users.Put(idKey(id), raw); err != nil {
			return err
		}
		return names.Put(usernameKey(nombre), idKey(id))
	})
	if err != nil {
		return nil, err
	}
	return &u, nil
}

func (d *DB) Usuario(id uint64) (*Usuario, error) {
	var u Usuario
	err := d.bolt.View(func(tx *bolt.Tx) error {
		raw := tx.Bucket([]byte(bucketUsers)).Get(idKey(id))
		if raw == nil {
			return ErrNoEncontrado
		}
		return json.Unmarshal(raw, &u)
	})
	if err != nil {
		return nil, err
	}
	return &u, nil
}

func (d *DB) UsuarioPorNombre(nombre string) (*Usuario, error) {
	var u Usuario
	err := d.bolt.View(func(tx *bolt.Tx) error {
		idRaw := tx.Bucket([]byte(bucketUsernames)).Get(usernameKey(nombre))
		if idRaw == nil {
			return ErrNoEncontrado
		}
		raw := tx.Bucket([]byte(bucketUsers)).Get(idRaw)
		if raw == nil {
			return ErrNoEncontrado
		}
		return json.Unmarshal(raw, &u)
	})
	if err != nil {
		return nil, err
	}
	return &u, nil
}

func (d *DB) GuardarUsuario(u *Usuario) error {
	return d.bolt.Update(func(tx *bolt.Tx) error {
		raw, err := json.Marshal(u)
		if err != nil {
			return err
		}
		return tx.Bucket([]byte(bucketUsers)).Put(idKey(u.ID), raw)
	})
}

func (d *DB) VerificarPassword(u *Usuario, password string) bool {
	if u == nil {
		return false
	}
	return checkPassword(u.Hash, password)
}

func (d *DB) CambiarPassword(u *Usuario, password string) error {
	hash, err := hashPassword(password)
	if err != nil {
		return err
	}
	u.Hash = hash
	return d.GuardarUsuario(u)
}

func (d *DB) BootstrapAdmin(nombre, password string) error {
	if nombre == "" || password == "" {
		return nil
	}
	existente, err := d.UsuarioPorNombre(nombre)
	if err == nil {
		hash, err := hashPassword(password)
		if err != nil {
			return err
		}
		existente.Hash = hash
		existente.Admin = true
		if existente.Display == "" {
			existente.Display = nombre
		}
		return d.GuardarUsuario(existente)
	}
	if err != ErrNoEncontrado {
		return err
	}
	_, err = d.CrearUsuario(nombre, nombre, password, true)
	return err
}
