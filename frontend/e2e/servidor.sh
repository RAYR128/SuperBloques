#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
DIST="$ROOT/frontend/editor/dist"
if [[ ! -f "$DIST/index.html" ]]; then
	echo "falta $DIST/index.html; corre pnpm build en frontend/editor" >&2
	exit 1
fi

TMP="$(mktemp -d)"
export ADDR="127.0.0.1:8099"
export DB_PATH="$TMP/superbloques.db"
export EDITOR_DIR="$DIST"
export SITIO_DIR="$ROOT/frontend/sitio"
cd "$ROOT/backend"
exec go run ./cmd/main
