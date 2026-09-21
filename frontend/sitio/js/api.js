export async function api(path, opts = {}) {
	const headers = { ...(opts.headers || {}) };
	const init = { credentials: "same-origin", ...opts, headers };
	if (opts.body !== undefined && typeof opts.body !== "string" && !(opts.body instanceof FormData)) {
		headers["Content-Type"] = "application/json";
		init.body = JSON.stringify(opts.body);
	}
	const res = await fetch(path, init);
	if (res.status === 204) {
		return null;
	}
	const text = await res.text();
	let data = null;
	if (text) {
		try {
			data = JSON.parse(text);
		} catch {
			data = { error: text };
		}
	}
	if (!res.ok) {
		const err = new Error((data && data.error) || res.statusText);
		err.status = res.status;
		err.data = data;
		throw err;
	}
	return data;
}

export async function me() {
	try {
		return await api("/api/me");
	} catch (e) {
		if (e.status === 401) {
			return null;
		}
		throw e;
	}
}

export function nextDestino() {
	const n = new URLSearchParams(location.search).get("next") || "/";
	if (!n.startsWith("/") || n.startsWith("//") || n.includes("://")) {
		return "/";
	}
	return n;
}

export function escapeHtml(s) {
	return String(s ?? "")
		.replaceAll("&", "&amp;")
		.replaceAll("<", "&lt;")
		.replaceAll(">", "&gt;")
		.replaceAll('"', "&quot;");
}

export function renderGaleria(el, proyectos, emptyMsg) {
	if (!proyectos || !proyectos.length) {
		el.innerHTML = `<p class="empty">${escapeHtml(emptyMsg)}</p>`;
		return;
	}
	el.innerHTML = `<div class="gallery">${proyectos.map(cardHtml).join("")}</div>`;
}

function cardHtml(p) {
	const autor = p.ownerDisplay || p.ownerNombre || "anonimo";
	const fecha = p.actualizado ? new Date(p.actualizado).toLocaleString("es") : "";
	return `<article class="card">
		<a href="/project/${p.id}"><img src="${escapeHtml(p.thumb)}" alt="" width="256" height="224" /></a>
		<div class="card-body">
			<a class="card-title" href="/project/${p.id}">${escapeHtml(p.nombre)}</a>
			<div class="card-meta">
				<a href="/u/${p.ownerId}">${escapeHtml(autor)}</a>
				${fecha ? " · " + escapeHtml(fecha) : ""}
			</div>
		</div>
	</article>`;
}
