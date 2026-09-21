import { api, nextDestino } from "./api.js";
import { mountNav } from "./nav.js";

const form = document.getElementById("auth-form");
const errorEl = document.getElementById("auth-error");
const modo = form?.dataset.modo || "login";

await mountNav(document.getElementById("nav"));

const next = new URLSearchParams(location.search).get("next");
if (next) {
	for (const sel of ['a[href="/register"]', 'a[href="/login"]']) {
		document.querySelectorAll(sel).forEach((a) => {
			const url = new URL(a.getAttribute("href"), location.origin);
			url.searchParams.set("next", next);
			a.setAttribute("href", url.pathname + url.search);
		});
	}
}

form?.addEventListener("submit", async (e) => {
	e.preventDefault();
	errorEl.textContent = "";
	const data = new FormData(form);
	const body = {
		nombre: String(data.get("nombre") || "").trim(),
		password: String(data.get("password") || ""),
	};
	if (modo === "register") {
		const display = String(data.get("display") || "").trim();
		if (display) {
			body.display = display;
		}
	}
	const btn = form.querySelector("button[type=submit]");
	if (btn) {
		btn.disabled = true;
	}
	try {
		const path = modo === "register" ? "/api/register" : "/api/login";
		await api(path, { method: "POST", body });
		location.href = nextDestino();
	} catch (err) {
		errorEl.textContent = err.message;
	} finally {
		if (btn) {
			btn.disabled = false;
		}
	}
});
