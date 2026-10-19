let seq = 0;

export const PASSWORD = "clave-1234";

export function nombreUnico() {
	seq += 1;
	return `pw${Date.now().toString(36)}${seq}`.slice(0, 24);
}

function cliente(destino) {
	if (destino && typeof destino.post === "function") {
		return destino;
	}
	return destino.request;
}

export async function registrar(destino, extra = {}) {
	const api = cliente(destino);
	const nombre = extra.nombre ?? nombreUnico();
	const display = extra.display ?? nombre;
	const password = extra.password ?? PASSWORD;
	const res = await api.post("/api/register", {
		data: {nombre, password, display},
	});
	if (!res.ok()) {
		throw new Error(`register ${res.status()} ${await res.text()}`);
	}
	const user = await res.json();
	return {...user, password};
}
