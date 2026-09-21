package db

import (
	"encoding/binary"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"time"

	bolt "go.etcd.io/bbolt"
	"golang.org/x/crypto/bcrypt"
)

const (
	bucketMeta         = "meta"
	bucketUsers        = "users"
	bucketUsernames    = "usernames"
	bucketSessions     = "sessions"
	bucketProjects     = "projects"
	bucketProjectData  = "project_data"
	bucketThumbs       = "thumbs"
	bucketUserProjects = "user_projects"
	bucketRecent       = "recent"

	keyNextUserID    = "next_user_id"
	keyNextProjectID = "next_project_id"
)

var (
	ErrNoEncontrado = errors.New("no encontrado")
	ErrDuplicado    = errors.New("duplicado")
	ErrExpirado     = errors.New("expirado")
)

type DB struct {
	bolt *bolt.DB
}

func Open(path string) (*DB, error) {
	dir := filepath.Dir(path)
	if dir != "" && dir != "." {
		if err := os.MkdirAll(dir, 0o755); err != nil {
			return nil, fmt.Errorf("crear dir db: %w", err)
		}
	}
	b, err := bolt.Open(path, 0o600, &bolt.Options{Timeout: 2 * time.Second})
	if err != nil {
		return nil, fmt.Errorf("abrir bbolt: %w", err)
	}
	d := &DB{bolt: b}
	if err := d.init(); err != nil {
		_ = b.Close()
		return nil, err
	}
	return d, nil
}

func (d *DB) Close() error {
	if d == nil || d.bolt == nil {
		return nil
	}
	return d.bolt.Close()
}

func (d *DB) init() error {
	return d.bolt.Update(func(tx *bolt.Tx) error {
		for _, name := range []string{
			bucketMeta,
			bucketUsers,
			bucketUsernames,
			bucketSessions,
			bucketProjects,
			bucketProjectData,
			bucketThumbs,
			bucketUserProjects,
			bucketRecent,
		} {
			if _, err := tx.CreateBucketIfNotExists([]byte(name)); err != nil {
				return err
			}
		}
		return nil
	})
}

func idKey(id uint64) []byte {
	b := make([]byte, 8)
	binary.BigEndian.PutUint64(b, id)
	return b
}

func nextID(tx *bolt.Tx, key string) (uint64, error) {
	meta := tx.Bucket([]byte(bucketMeta))
	raw := meta.Get([]byte(key))
	var n uint64 = 1
	if len(raw) == 8 {
		n = binary.BigEndian.Uint64(raw)
	}
	if err := meta.Put([]byte(key), idKey(n+1)); err != nil {
		return 0, err
	}
	return n, nil
}

func hashPassword(password string) ([]byte, error) {
	return bcrypt.GenerateFromPassword([]byte(password), 12)
}

func checkPassword(hash []byte, password string) bool {
	return bcrypt.CompareHashAndPassword(hash, []byte(password)) == nil
}
