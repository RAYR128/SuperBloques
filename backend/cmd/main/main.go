package main

import (
	"log"
	"net/http"
	"os"

	"superbloques/sitio"
)

func main() {
	addr := ":8080"
	if v := os.Getenv("ADDR"); v != "" {
		addr = v
	}

	h, err := sitio.New()
	if err != nil {
		log.Fatal(err)
	}

	log.Printf("escuchando en %s", addr)
	if err := http.ListenAndServe(addr, h); err != nil {
		log.Fatal(err)
	}
}
