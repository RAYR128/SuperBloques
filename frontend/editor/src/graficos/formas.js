export function linea(x0, y0, x1, y1, plot) {
	const dx = Math.abs(x1 - x0);
	const dy = Math.abs(y1 - y0);
	const sx = x0 < x1 ? 1 : -1;
	const sy = y0 < y1 ? 1 : -1;
	let err = dx - dy;
	for (;;) {
		plot(x0, y0);
		if (x0 === x1 && y0 === y1) {
			return;
		}
		const e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y0 += sy;
		}
	}
}

export function rectangulo(x0, y0, x1, y1, plot) {
	const izq = Math.min(x0, x1);
	const der = Math.max(x0, x1);
	const arriba = Math.min(y0, y1);
	const abajo = Math.max(y0, y1);
	for (let x = izq; x <= der; x++) {
		plot(x, arriba);
		if (abajo !== arriba) {
			plot(x, abajo);
		}
	}
	for (let y = arriba + 1; y < abajo; y++) {
		plot(izq, y);
		if (der !== izq) {
			plot(der, y);
		}
	}
}

export function rectanguloRelleno(x0, y0, x1, y1, plot) {
	const izq = Math.min(x0, x1);
	const der = Math.max(x0, x1);
	const arriba = Math.min(y0, y1);
	const abajo = Math.max(y0, y1);
	for (let y = arriba; y <= abajo; y++) {
		for (let x = izq; x <= der; x++) {
			plot(x, y);
		}
	}
}

function visitarCirculo(r, visit) {
	if (r <= 0) {
		visit(0, 0);
		return;
	}
	let x = r;
	let y = 0;
	let d = 1 - r;
	while (x >= y) {
		visit(x, y);
		y++;
		if (d <= 0) {
			d += 2 * y + 1;
		} else {
			x--;
			d += 2 * (y - x) + 1;
		}
	}
}

function trazarOctantes(cx, cy, x, y, plot) {
	plot(cx + x, cy + y);
	plot(cx + x, cy - y);
	plot(cx - x, cy + y);
	plot(cx - x, cy - y);
	plot(cx + y, cy + x);
	plot(cx + y, cy - x);
	plot(cx - y, cy + x);
	plot(cx - y, cy - x);
}

export function circulo(cx, cy, r, plot) {
	visitarCirculo(r, (x, y) => trazarOctantes(cx, cy, x, y, plot));
}

export function circuloRelleno(cx, cy, r, plot) {
	if (r <= 0) {
		plot(cx, cy);
		return;
	}
	const tramos = new Map();
	const tramo = (y, x0, x1) => {
		const previo = tramos.get(y);
		if (!previo) {
			tramos.set(y, [x0, x1]);
			return;
		}
		previo[0] = Math.min(previo[0], x0);
		previo[1] = Math.max(previo[1], x1);
	};
	visitarCirculo(r, (x, y) => {
		tramo(cy + y, cx - x, cx + x);
		tramo(cy - y, cx - x, cx + x);
		tramo(cy + x, cx - y, cx + y);
		tramo(cy - x, cx - y, cx + y);
	});
	for (const [y, [x0, x1]] of tramos) {
		for (let x = x0; x <= x1; x++) {
			plot(x, y);
		}
	}
}
