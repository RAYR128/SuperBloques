function escapeText(s) {
	return String(s ?? "")
		.replaceAll("&", "&amp;")
		.replaceAll("<", "&lt;")
		.replaceAll(">", "&gt;")
		.replaceAll('"', "&quot;");
}

export async function usuarioActual() {
	try {
		const res = await fetch("/api/me", { credentials: "same-origin" });
		if (!res.ok) {
			return null;
		}
		return await res.json();
	} catch {
		return null;
	}
}

export async function pintarHeaderUser() {
	const el = document.getElementById("header-user");
	if (!el) {
		return null;
	}
	const user = await usuarioActual();
	if (user) {
		el.innerHTML = `<a href="/account">${escapeText(user.display || user.nombre)}</a>`;
	} else {
		const next = encodeURIComponent(location.pathname + location.search);
		el.innerHTML = `<a href="/login?next=${next}">Entrar</a>`;
	}
	return user;
}
