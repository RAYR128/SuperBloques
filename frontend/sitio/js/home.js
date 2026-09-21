import { api, renderGaleria } from "./api.js";
import { mountNav } from "./nav.js";

const nav = document.getElementById("nav");
const gallery = document.getElementById("gallery");

await mountNav(nav);

try {
	const proyectos = await api("/api/projects");
	renderGaleria(gallery, proyectos, "Todavia no hay proyectos publicados.");
} catch (e) {
	gallery.innerHTML = `<p class="error">${e.message}</p>`;
}
