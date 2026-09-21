package db

import (
	"bytes"
	"encoding/binary"
	"encoding/json"
	"sort"
	"time"

	bolt "go.etcd.io/bbolt"
)

type ProyectoMeta struct {
	ID          uint64    `json:"id"`
	OwnerID     uint64    `json:"ownerId"`
	Nombre      string    `json:"nombre"`
	Creado      time.Time `json:"creado"`
	Actualizado time.Time `json:"actualizado"`
}

func recentKey(t time.Time, id uint64) []byte {
	k := make([]byte, 16)
	ns := uint64(t.UnixNano())
	binary.BigEndian.PutUint64(k[0:8], ^ns)
	binary.BigEndian.PutUint64(k[8:16], ^id)
	return k
}

func userProjectKey(userID, projectID uint64) []byte {
	k := make([]byte, 16)
	binary.BigEndian.PutUint64(k[0:8], userID)
	binary.BigEndian.PutUint64(k[8:16], projectID)
	return k
}

func putMeta(tx *bolt.Tx, p ProyectoMeta) error {
	raw, err := json.Marshal(p)
	if err != nil {
		return err
	}
	return tx.Bucket([]byte(bucketProjects)).Put(idKey(p.ID), raw)
}

func getMeta(tx *bolt.Tx, id uint64) (*ProyectoMeta, error) {
	raw := tx.Bucket([]byte(bucketProjects)).Get(idKey(id))
	if raw == nil {
		return nil, ErrNoEncontrado
	}
	var p ProyectoMeta
	if err := json.Unmarshal(raw, &p); err != nil {
		return nil, err
	}
	return &p, nil
}

func (d *DB) CrearProyecto(ownerID uint64, nombre string, data, thumb []byte) (*ProyectoMeta, error) {
	now := time.Now().UTC()
	var p ProyectoMeta
	err := d.bolt.Update(func(tx *bolt.Tx) error {
		id, err := nextID(tx, keyNextProjectID)
		if err != nil {
			return err
		}
		p = ProyectoMeta{
			ID:          id,
			OwnerID:     ownerID,
			Nombre:      nombre,
			Creado:      now,
			Actualizado: now,
		}
		if err := putMeta(tx, p); err != nil {
			return err
		}
		if err := tx.Bucket([]byte(bucketProjectData)).Put(idKey(id), data); err != nil {
			return err
		}
		if len(thumb) > 0 {
			if err := tx.Bucket([]byte(bucketThumbs)).Put(idKey(id), thumb); err != nil {
				return err
			}
		}
		if err := tx.Bucket([]byte(bucketUserProjects)).Put(userProjectKey(ownerID, id), []byte{1}); err != nil {
			return err
		}
		return tx.Bucket([]byte(bucketRecent)).Put(recentKey(now, id), []byte{1})
	})
	if err != nil {
		return nil, err
	}
	return &p, nil
}

func (d *DB) Proyecto(id uint64) (*ProyectoMeta, error) {
	var p *ProyectoMeta
	err := d.bolt.View(func(tx *bolt.Tx) error {
		var err error
		p, err = getMeta(tx, id)
		return err
	})
	if err != nil {
		return nil, err
	}
	return p, nil
}

func (d *DB) ProyectoDatos(id uint64) ([]byte, error) {
	var data []byte
	err := d.bolt.View(func(tx *bolt.Tx) error {
		raw := tx.Bucket([]byte(bucketProjectData)).Get(idKey(id))
		if raw == nil {
			return ErrNoEncontrado
		}
		data = append([]byte(nil), raw...)
		return nil
	})
	if err != nil {
		return nil, err
	}
	return data, nil
}

func (d *DB) Thumb(id uint64) ([]byte, error) {
	var data []byte
	err := d.bolt.View(func(tx *bolt.Tx) error {
		raw := tx.Bucket([]byte(bucketThumbs)).Get(idKey(id))
		if raw == nil {
			return ErrNoEncontrado
		}
		data = append([]byte(nil), raw...)
		return nil
	})
	if err != nil {
		return nil, err
	}
	return data, nil
}

func (d *DB) ActualizarProyecto(id uint64, nombre *string, data, thumb []byte) (*ProyectoMeta, error) {
	now := time.Now().UTC()
	var p ProyectoMeta
	err := d.bolt.Update(func(tx *bolt.Tx) error {
		old, err := getMeta(tx, id)
		if err != nil {
			return err
		}
		p = *old
		if nombre != nil {
			p.Nombre = *nombre
		}
		oldKey := recentKey(p.Actualizado, id)
		p.Actualizado = now
		if err := putMeta(tx, p); err != nil {
			return err
		}
		if data != nil {
			if err := tx.Bucket([]byte(bucketProjectData)).Put(idKey(id), data); err != nil {
				return err
			}
		}
		if len(thumb) > 0 {
			if err := tx.Bucket([]byte(bucketThumbs)).Put(idKey(id), thumb); err != nil {
				return err
			}
		}
		recent := tx.Bucket([]byte(bucketRecent))
		if err := recent.Delete(oldKey); err != nil {
			return err
		}
		return recent.Put(recentKey(now, id), []byte{1})
	})
	if err != nil {
		return nil, err
	}
	return &p, nil
}

func (d *DB) BorrarProyecto(id uint64) error {
	return d.bolt.Update(func(tx *bolt.Tx) error {
		p, err := getMeta(tx, id)
		if err != nil {
			return err
		}
		if err := tx.Bucket([]byte(bucketProjects)).Delete(idKey(id)); err != nil {
			return err
		}
		if err := tx.Bucket([]byte(bucketProjectData)).Delete(idKey(id)); err != nil {
			return err
		}
		if err := tx.Bucket([]byte(bucketThumbs)).Delete(idKey(id)); err != nil {
			return err
		}
		if err := tx.Bucket([]byte(bucketUserProjects)).Delete(userProjectKey(p.OwnerID, id)); err != nil {
			return err
		}
		return tx.Bucket([]byte(bucketRecent)).Delete(recentKey(p.Actualizado, id))
	})
}

func (d *DB) ProyectosRecientes(n int) ([]ProyectoMeta, error) {
	if n <= 0 {
		return nil, nil
	}
	out := make([]ProyectoMeta, 0, n)
	err := d.bolt.View(func(tx *bolt.Tx) error {
		c := tx.Bucket([]byte(bucketRecent)).Cursor()
		projects := tx.Bucket([]byte(bucketProjects))
		for k, _ := c.First(); k != nil && len(out) < n; k, _ = c.Next() {
			if len(k) < 16 {
				continue
			}
			id := ^binary.BigEndian.Uint64(k[8:16])
			raw := projects.Get(idKey(id))
			if raw == nil {
				continue
			}
			var p ProyectoMeta
			if err := json.Unmarshal(raw, &p); err != nil {
				return err
			}
			out = append(out, p)
		}
		return nil
	})
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (d *DB) ProyectosDeUsuario(userID uint64) ([]ProyectoMeta, error) {
	var out []ProyectoMeta
	prefix := idKey(userID)
	err := d.bolt.View(func(tx *bolt.Tx) error {
		c := tx.Bucket([]byte(bucketUserProjects)).Cursor()
		projects := tx.Bucket([]byte(bucketProjects))
		for k, _ := c.Seek(prefix); k != nil && bytes.HasPrefix(k, prefix); k, _ = c.Next() {
			if len(k) < 16 {
				continue
			}
			id := binary.BigEndian.Uint64(k[8:16])
			raw := projects.Get(idKey(id))
			if raw == nil {
				continue
			}
			var p ProyectoMeta
			if err := json.Unmarshal(raw, &p); err != nil {
				return err
			}
			out = append(out, p)
		}
		return nil
	})
	if err != nil {
		return nil, err
	}
	sort.Slice(out, func(i, j int) bool {
		return out[i].Actualizado.After(out[j].Actualizado)
	})
	return out, nil
}
