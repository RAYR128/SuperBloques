set -e
./compilar_wasm.sh
cd frontend/editor
pnpm build
cd ../../backend
go run ./cmd/main