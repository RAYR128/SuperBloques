package sitio

import (
	"fmt"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"strings"

	"superbloques/db"

	"github.com/go-chi/chi/v5"
	"github.com/go-chi/chi/v5/middleware"
)

type Server struct {
	db        *db.DB
	editorDir string
	sitioDir  string
	mux       *chi.Mux
	secureCK  bool
}

func New() (http.Handler, error) {
	editorDir, err := resolveEditorDir()
	if err != nil {
		return nil, err
	}
	sitioDir, err := resolveSitioDir()
	if err != nil {
		return nil, err
	}
	dbPath := os.Getenv("DB_PATH")
	if dbPath == "" {
		dbPath = filepath.Join("data", "superbloques.db")
	}
	database, err := db.Open(dbPath)
	if err != nil {
		return nil, err
	}
	adminUser := strings.TrimSpace(os.Getenv("SUPERBLOQUES_ADMIN_USER"))
	adminPass := os.Getenv("SUPERBLOQUES_ADMIN_PASSWORD")
	if (adminUser == "") != (adminPass == "") {
		log.Print("SUPERBLOQUES_ADMIN_USER y SUPERBLOQUES_ADMIN_PASSWORD deben ir juntos; se ignora el bootstrap")
	} else if adminUser != "" {
		if err := database.BootstrapAdmin(adminUser, adminPass); err != nil {
			_ = database.Close()
			return nil, fmt.Errorf("bootstrap admin: %w", err)
		}
		log.Printf("cuenta admin lista: %s", adminUser)
	}
	return NewWith(database, editorDir, sitioDir), nil
}

func NewWith(database *db.DB, editorDir, sitioDir string) http.Handler {
	s := &Server{
		db:        database,
		editorDir: editorDir,
		sitioDir:  sitioDir,
		secureCK:  os.Getenv("TRUST_HTTPS") == "1",
	}
	s.routes()
	return s
}

func (s *Server) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	s.mux.ServeHTTP(w, r)
}

func (s *Server) routes() {
	r := chi.NewRouter()
	r.Use(middleware.RequestID)
	r.Use(middleware.RealIP)
	r.Use(middleware.Logger)
	r.Use(middleware.Recoverer)
	r.Use(maxBytes(8 << 20))
	r.Use(s.withSession)

	r.Get("/api/health", health)

	r.Post("/api/register", s.register)
	r.Post("/api/login", s.login)
	r.Get("/api/me", s.me)
	r.Get("/api/user/{id}", s.getUser)
	r.Get("/api/user/{id}/projects", s.listUserProjects)
	r.Get("/api/projects", s.listProjects)
	r.Get("/api/project/{id}", s.getProject)

	r.Get("/", s.serveSitio("index.html"))
	r.Get("/login", s.pageLogin)
	r.Get("/register", s.pageRegister)
	r.Get("/account", s.pageAccount)
	r.Get("/u/{id}", s.serveSitio("user.html"))
	r.Get("/project/{id}/thumb", s.projectThumb)
	r.Get("/project/{id}", s.pageProject)

	r.Group(func(r chi.Router) {
		r.Use(s.requireAuthJSON)
		r.Post("/api/logout", s.logout)
		r.Post("/api/account", s.updateAccount)
		r.Post("/project", s.createProject)
		r.Post("/project/{id}", s.updateProject)
		r.Delete("/project/{id}", s.deleteProject)
	})

	fileServer(r, "/editor", http.Dir(s.editorDir))
	fileServer(r, "/sitio", http.Dir(s.sitioDir))

	s.mux = r
}

func health(w http.ResponseWriter, r *http.Request) {
	writeJSON(w, http.StatusOK, map[string]string{"status": "ok"})
}

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
	return firstDir([]string{
		filepath.Join("frontend", "editor", "dist"),
		filepath.Join("..", "frontend", "editor", "dist"),
	}, "no se encontro frontend/editor/dist; corre pnpm build en frontend/editor")
}

func resolveSitioDir() (string, error) {
	if env := os.Getenv("SITIO_DIR"); env != "" {
		abs, err := filepath.Abs(env)
		if err != nil {
			return "", fmt.Errorf("SITIO_DIR: %w", err)
		}
		if err := mustDir(abs); err != nil {
			return "", err
		}
		return abs, nil
	}
	return firstDir([]string{
		filepath.Join("frontend", "sitio"),
		filepath.Join("..", "frontend", "sitio"),
	}, "no se encontro frontend/sitio")
}

func firstDir(candidatos []string, msg string) (string, error) {
	for _, c := range candidatos {
		abs, err := filepath.Abs(c)
		if err != nil {
			continue
		}
		if err := mustDir(abs); err == nil {
			return abs, nil
		}
	}
	return "", fmt.Errorf("%s (cwd=%s)", msg, mustCwd())
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
