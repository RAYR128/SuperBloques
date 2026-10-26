// Arranca el backend de los tests e2e. Playwright lo lanza en Windows y en Unix, no depende de bash. Misma base vacia y mismo puerto que servidor.sh.
import {spawn} from "node:child_process";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import {fileURLToPath} from "node:url";

const aqui = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(aqui, "..", "..");
const dist = path.join(root, "frontend", "editor", "dist");

if (!fs.existsSync(path.join(dist, "index.html"))) {
	console.error(`falta ${path.join(dist, "index.html")}; corre pnpm build en frontend/editor`);
	process.exit(1);
}

const tmp = fs.mkdtempSync(path.join(os.tmpdir(), "superbloques-e2e-"));
const hijo = spawn("go", ["run", "./cmd/main"], {
	cwd: path.join(root, "backend"),
	stdio: "inherit",
	env: {
		...process.env,
		ADDR: "127.0.0.1:8099",
		DB_PATH: path.join(tmp, "superbloques.db"),
		EDITOR_DIR: dist,
		SITIO_DIR: path.join(root, "frontend", "sitio"),
	},
});

let cerrando = false;

function matarHijo() {
	if (!hijo.pid || hijo.exitCode !== null) return;
	if (process.platform === "win32") {
		spawn("taskkill", ["/pid", String(hijo.pid), "/T", "/F"], {stdio: "ignore"});
		return;
	}
	hijo.kill("SIGTERM");
}

function cerrar(codigo) {
	if (cerrando) return;
	cerrando = true;
	matarHijo();
	hijo.once("exit", () => process.exit(codigo));
}

hijo.on("error", (err) => {
	console.error(err.code === "ENOENT" ? "no se encontro go en el PATH" : err.message);
	cerrando = true;
	process.exit(1);
});

hijo.on("exit", (code, signal) => {
	try {
		fs.rmSync(tmp, {recursive: true, force: true});
	} catch {
		// En Windows el archivo de la base puede seguir bloqueado un instante.
	}
	if (!cerrando) process.exit(code ?? (signal ? 1 : 0));
});

process.on("SIGINT", () => cerrar(130));
process.on("SIGTERM", () => cerrar(143));
