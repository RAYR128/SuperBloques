import {initEmu} from "./emu.js";
import {initMenu} from "./menu.js";
import {initTree} from "./tree.js";
import {initSplitters} from "./splitters.js";
import {initWorkspace} from "./blocks/workspace.js";
import {parsearProyecto} from "./formato.js";
import {cargarProyecto} from "./project.js";
import {pintarHeaderUser} from "./session.js";

function info(html) {
	const el = document.querySelector(".debug-body");
	if (el) {
		el.innerHTML = html;
	}
}

window.addEventListener("error", (e) => {
	info(`<div class="warn">[error] ${e.message}</div><div class="dim">${e.filename}:${e.lineno}</div>`);
});

try {
	initSplitters();
	initWorkspace();
	initTree();
	initMenu();
	initEmu();
	void pintarHeaderUser();
	const params = new URLSearchParams(location.search);
	const cat = params.get("cat");
	if (cat) {
		document.querySelector(`.cat[data-cat="${cat}"]`)?.click();
	}
	if (params.has("menu")) {
		document.getElementById("menu-file")?.click();
	}
	info('<div class="ok">[ok] editor listo</div>');
	void cargarDesdeUrl(info);
} catch (e) {
	console.error(e);
	info(`<div class="warn">[error] ${e.message}</div><div class="dim">${e.stack ?? ""}</div>`);
}

function idDeUrl() {
	const m = location.pathname.match(/\/project\/(\d+)/);
	if (m) {
		return m[1];
	}
	return new URLSearchParams(location.search).get("project");
}

async function cargarDesdeUrl(info) {
	const id = idDeUrl();
	if (!id) {
		return;
	}
	try {
		const res = await fetch(`/api/project/${id}`, { credentials: "same-origin" });
		const data = await res.json().catch(() => ({}));
		if (!res.ok) {
			info(`<div class="warn">[error] no se pudo cargar el proyecto</div><div class="dim">${data.error || res.status}</div>`);
			return;
		}
		const proyecto = parsearProyecto(JSON.stringify(data.proyecto));
		cargarProyecto(proyecto, `${data.nombre}.json`, data.id);
		info(`<div class="ok">[ok] proyecto cargado</div><div class="dim">${data.nombre} (#${data.id})</div>`);
	} catch (e) {
		const msg = e instanceof Error ? e.message : String(e);
		info(`<div class="warn">[error] ${msg}</div><div class="dim">el proyecto en memoria no se modifico</div>`);
	}
}
