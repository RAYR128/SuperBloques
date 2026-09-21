import { api, escapeHtml, renderGaleria } from "./api.js";
import { mountNav } from "./nav.js";

await mountNav(document.getElementById("nav"));

const parts = location.pathname.split("/").filter(Boolean);
const id = parts[1];
const head = document.getElementById("profile-head");
const gallery = document.getElementById("gallery");

if (!id) {
	head.innerHTML = `<p class="error">Usuario no encontrado</p>`;
} else {
	try {
		const u = await api(`/api/user/${id}`);
		document.title = `${u.display || u.nombre} - SuperBloques`;
		const creado = u.creado ? new Date(u.creado).toLocaleDateString("es") : "";
		head.innerHTML = `
			<h1>${escapeHtml(u.display || u.nombre)}</h1>
			<div class="meta">
				<span>@${escapeHtml(u.nombre)}</span>
				${creado ? `<span>desde ${escapeHtml(creado)}</span>` : ""}
				${u.admin ? `<span class="badge">admin</span>` : ""}
			</div>
		`;
		const proyectos = await api(`/api/user/${id}/projects`);
		renderGaleria(gallery, proyectos, "Este usuario todavia no publico proyectos.");
	} catch (e) {
		head.innerHTML = `<p class="error">${e.status === 404 ? "Usuario no encontrado" : e.message}</p>`;
	}
}
