import { api, me } from "./api.js";

export async function mountNav(el) {
	if (!el) {
		return null;
	}
	let user = null;
	try {
		user = await me();
	} catch {
		user = null;
	}

	const cuenta = user
		? `<a href="/u/${user.id}">${escapeText(user.display || user.nombre)}</a>
			<a href="/account">Cuenta</a>
			<button type="button" id="nav-logout">Salir</button>`
		: `<a href="/login">Entrar</a>
			<a href="/register">Crear cuenta</a>`;

	el.innerHTML = `
		<a class="logo" href="/">SuperBloques</a>
		<div class="links">
			<a href="/editor/">Editor</a>
			${cuenta}
		</div>
	`;

	el.querySelector("#nav-logout")?.addEventListener("click", async () => {
		try {
			await api("/api/logout", { method: "POST" });
		} catch {
			/* ignorar */
		}
		location.href = "/";
	});
	return user;
}

function escapeText(s) {
	return String(s ?? "")
		.replaceAll("&", "&amp;")
		.replaceAll("<", "&lt;")
		.replaceAll(">", "&gt;")
		.replaceAll('"', "&quot;");
}
