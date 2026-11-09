# syntax=docker/dockerfile:1

FROM emscripten/emsdk:5.0.7 AS wasm
WORKDIR /src
COPY . .
ENV EMSDK_PATH=/emsdk
ENV EMSDK_QUIET=1
RUN bash ./compilar_wasm.sh

FROM node:22-bookworm AS frontend
WORKDIR /src/frontend/editor
ENV COREPACK_ENABLE_DOWNLOAD_PROMPT=0
RUN corepack enable && corepack prepare pnpm@11.1.2 --activate
COPY frontend/editor/package.json frontend/editor/pnpm-lock.yaml frontend/editor/pnpm-workspace.yaml ./
RUN pnpm install --frozen-lockfile
COPY frontend/editor/ ./
COPY --from=wasm /src/frontend/editor/public/compilador ./public/compilador
COPY --from=wasm /src/frontend/editor/public/emu ./public/emu
RUN pnpm build

FROM golang:1.26.2-bookworm AS backend
WORKDIR /src/backend
COPY backend/go.mod backend/go.sum ./
RUN go mod download
COPY backend/ ./
RUN CGO_ENABLED=0 GOOS=linux go build -o /out/superbloques ./cmd/main

FROM debian:bookworm-slim AS runtime
RUN apt-get update \
	&& apt-get install -y --no-install-recommends ca-certificates curl \
	&& rm -rf /var/lib/apt/lists/* \
	&& mkdir -p /data /app/editor /app/sitio
COPY --from=backend /out/superbloques /app/superbloques
COPY --from=frontend /src/frontend/editor/dist /app/editor
COPY frontend/sitio /app/sitio
ENV ADDR=:80 \
	DB_PATH=/data/superbloques.db \
	EDITOR_DIR=/app/editor \
	SITIO_DIR=/app/sitio
EXPOSE 80
VOLUME /data
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
	CMD curl -fsS http://127.0.0.1/api/health || exit 1
ENTRYPOINT ["/app/superbloques"]
