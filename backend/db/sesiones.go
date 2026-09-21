package db

import (
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"time"

	bolt "go.etcd.io/bbolt"
)

type Sesion struct {
	UserID uint64    `json:"userId"`
	Expira time.Time `json:"expira"`
}

func nuevoToken() (string, error) {
	var b [32]byte
	if _, err := rand.Read(b[:]); err != nil {
		return "", err
	}
	return hex.EncodeToString(b[:]), nil
}

func (d *DB) CrearSesion(userID uint64, ttl time.Duration) (string, error) {
	token, err := nuevoToken()
	if err != nil {
		return "", err
	}
	s := Sesion{
		UserID: userID,
		Expira: time.Now().UTC().Add(ttl),
	}
	raw, err := json.Marshal(s)
	if err != nil {
		return "", err
	}
	err = d.bolt.Update(func(tx *bolt.Tx) error {
		return tx.Bucket([]byte(bucketSessions)).Put([]byte(token), raw)
	})
	if err != nil {
		return "", err
	}
	return token, nil
}

func (d *DB) Sesion(token string) (*Sesion, error) {
	if token == "" {
		return nil, ErrNoEncontrado
	}
	var s Sesion
	var expirada bool
	err := d.bolt.View(func(tx *bolt.Tx) error {
		raw := tx.Bucket([]byte(bucketSessions)).Get([]byte(token))
		if raw == nil {
			return ErrNoEncontrado
		}
		if err := json.Unmarshal(raw, &s); err != nil {
			return err
		}
		if !s.Expira.After(time.Now().UTC()) {
			expirada = true
		}
		return nil
	})
	if err != nil {
		return nil, err
	}
	if expirada {
		_ = d.BorrarSesion(token)
		return nil, ErrExpirado
	}
	return &s, nil
}

func (d *DB) BorrarSesion(token string) error {
	if token == "" {
		return nil
	}
	return d.bolt.Update(func(tx *bolt.Tx) error {
		return tx.Bucket([]byte(bucketSessions)).Delete([]byte(token))
	})
}
