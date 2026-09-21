import { flushWorkspace } from "./blocks/workspace.js";
import { marcarPublicado, state, tituloProyecto } from "./project.js";
import { usuarioActual } from "./session.js";

function informar(html) {
	const el = document.querySelector(".debug-body");
	if (el) {
		el.innerHTML = html;
	}
}

function thumbnailDelStage() {
	const canvas = document.getElementById("stage");
	if (!canvas || typeof canvas.toDataURL !== "function") {
		return "";
	}
	try {
		return canvas.toDataURL("image/jpeg", 0.72);
	} catch {
		return "";
	}
}

function urlDeProyecto(id) {
	if (location.pathname.startsWith("/editor")) {
		return `/editor/?project=${id}`;
	}
	return `/project/${id}`;
}

export function initPublish() {
	const modal = document.getElementById("publish-modal");
	const nameInput = document.getElementById("publish-name");
	const title = document.getElementById("publish-title");
	const submit = document.getElementById("publish-submit");
	const cancel = document.getElementById("publish-cancel");
	const errorEl = document.getElementById("publish-error");
	if (!modal || !nameInput || !submit) {
		return;
	}

	function cerrar() {
		modal.hidden = true;
		if (errorEl) {
			errorEl.hidden = true;
			errorEl.textContent = "";
		}
	}

	function mostrarError(msg) {
		if (!errorEl) {
			window.alert(msg);
			return;
		}
		errorEl.textContent = msg;
		errorEl.hidden = false;
	}

	async function abrir() {
		const user = await usuarioActual();
		if (!user) {
			location.href = "/login?next=" + encodeURIComponent(location.pathname + location.search);
			return;
		}
		if (errorEl) {
			errorEl.hidden = true;
			errorEl.textContent = "";
		}
		nameInput.value = tituloProyecto();
		if (state.projectId) {
			if (title) {
				title.textContent = "Actualizar proyecto";
			}
			submit.textContent = "Actualizar";
		} else {
			if (title) {
				title.textContent = "Publicar proyecto";
			}
			submit.textContent = "Publicar";
		}
		modal.hidden = false;
		nameInput.focus();
		nameInput.select();
	}

	async function enviar() {
		const nombre = nameInput.value.trim();
		if (!nombre) {
			mostrarError("El nombre no puede estar vacio");
			return;
		}
		flushWorkspace();
		const body = {
			nombre,
			proyecto: state.proyecto,
			thumbnail: thumbnailDelStage(),
		};
		const update = Boolean(state.projectId);
		const url = update ? `/project/${state.projectId}` : "/project";
		submit.disabled = true;
		try {
			const res = await fetch(url, {
				method: "POST",
				credentials: "same-origin",
				headers: { "Content-Type": "application/json" },
				body: JSON.stringify(body),
			});
			const data = await res.json().catch(() => ({}));
			if (res.status === 401) {
				location.href = "/login?next=" + encodeURIComponent(location.pathname + location.search);
				return;
			}
			if (!res.ok) {
				mostrarError(data.error || "No se pudo publicar");
				return;
			}
			marcarPublicado(data.id, data.nombre || nombre);
			const dest = urlDeProyecto(data.id);
			if (dest !== location.pathname + location.search) {
				history.replaceState(null, "", dest);
			}
			cerrar();
			informar(`<div class="ok">[ok] proyecto ${update ? "actualizado" : "publicado"}</div><div class="dim">id ${data.id}</div>`);
		} catch (e) {
			const msg = e instanceof Error ? e.message : String(e);
			mostrarError(msg);
		} finally {
			submit.disabled = false;
		}
	}

	submit.addEventListener("click", () => {
		void enviar();
	});
	cancel?.addEventListener("click", cerrar);
	nameInput.addEventListener("keydown", (e) => {
		if (e.key === "Enter") {
			e.preventDefault();
			void enviar();
		}
	});
	modal.addEventListener("click", (e) => {
		if (e.target === modal) {
			cerrar();
		}
	});
	document.addEventListener("keydown", (e) => {
		if (e.key === "Escape" && !modal.hidden) {
			cerrar();
		}
	});

	return abrir;
}
