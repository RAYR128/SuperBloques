package sitio

import (
	"encoding/json"
	"fmt"
	"net/http"
	"os"
	"path/filepath"
	"strings"

	"github.com/go-chi/chi/v5"
	"github.com/go-chi/chi/v5/middleware"
)

// new arma el router con /api/health y el directorio estatico de /editor.
func New() (http.Handler, error) {
	editorDir, err := resolveEditorDir()
	if err != nil {
		return nil, err
	}

	r := chi.NewRouter()
	r.Use(middleware.RequestID)
	r.Use(middleware.RealIP)
	r.Use(middleware.Logger)
	r.Use(middleware.Recoverer)

	r.Get("/api/health", health)

	fileServer(r, "/editor", http.Dir(editorDir))

	return r, nil
}

func health(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	_ = json.NewEncoder(w).Encode(map[string]string{"status": "ok"})
}

// temporal hasta que se decida una ubicacion real
func resolveEditorDir() (string, error) {
	if env := os.Getenv("EDITOR_DIR"); env != "" {
		abs, err := filepath.Abs(env)
		if err != nil {
			return "", fmt.Errorf("EDITOR_DIR: %w", err)
		}
		if err := mustDir(abs); err != nil {
			return "", err
		}
		return abs, nil
	}

	candidatos := []string{
		filepath.Join("frontend", "editor"),
		filepath.Join("..", "frontend", "editor"),
	}
	for _, c := range candidatos {
		abs, err := filepath.Abs(c)
		if err != nil {
			continue
		}
		if err := mustDir(abs); err == nil {
			return abs, nil
		}
	}

	return "", fmt.Errorf("no se encontro frontend/editor (cwd=%s)", mustCwd())
}

func mustDir(path string) error {
	info, err := os.Stat(path)
	if err != nil {
		return err
	}
	if !info.IsDir() {
		return fmt.Errorf("%s no es un directorio", path)
	}
	return nil
}

func mustCwd() string {
	cwd, err := os.Getwd()
	if err != nil {
		return "?"
	}
	return cwd
}

// fileServer sirve un directorio bajo path, con redirect de /path a /path/.
func fileServer(r chi.Router, path string, root http.FileSystem) {
	if strings.ContainsAny(path, "{}*") {
		panic("fileServer no admite parametros en la ruta")
	}

	if path != "/" && path[len(path)-1] != '/' {
		r.Get(path, http.RedirectHandler(path+"/", http.StatusMovedPermanently).ServeHTTP)
		path += "/"
	}
	path += "*"

	r.Get(path, func(w http.ResponseWriter, req *http.Request) {
		rctx := chi.RouteContext(req.Context())
		prefix := strings.TrimSuffix(rctx.RoutePattern(), "/*")
		http.StripPrefix(prefix, http.FileServer(root)).ServeHTTP(w, req)
	})
}
