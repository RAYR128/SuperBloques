import { api } from "./api.js";
import { mountNav } from "./nav.js";

const form = document.getElementById("account-form");
const errorEl = document.getElementById("account-error");
const okEl = document.getElementById("account-ok");
const displayInput = document.getElementById("display");
const userLabel = document.getElementById("username");

const user = await mountNav(document.getElementById("nav"));
if (!user) {
	location.href = "/login?next=/account";
} else {
	if (displayInput) {
		displayInput.value = user.display || user.nombre;
	}
	if (userLabel) {
		userLabel.textContent = user.nombre;
	}
}

form?.addEventListener("submit", async (e) => {
	e.preventDefault();
	errorEl.textContent = "";
	okEl.textContent = "";
	const data = new FormData(form);
	const body = {
		display: String(data.get("display") || "").trim(),
		passwordActual: String(data.get("passwordActual") || ""),
		passwordNueva: String(data.get("passwordNueva") || ""),
	};
	const btn = form.querySelector("button[type=submit]");
	if (btn) {
		btn.disabled = true;
	}
	try {
		const updated = await api("/api/account", { method: "POST", body });
		okEl.textContent = "Cambios guardados";
		form.passwordActual.value = "";
		form.passwordNueva.value = "";
		if (updated?.display && displayInput) {
			displayInput.value = updated.display;
		}
		await mountNav(document.getElementById("nav"));
	} catch (err) {
		errorEl.textContent = err.message;
	} finally {
		if (btn) {
			btn.disabled = false;
		}
	}
});
