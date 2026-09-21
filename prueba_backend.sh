set -e
cd frontend/editor
pnpm build
cd ../../backend
go run ./cmd/main