const STORAGE_KEY = "superbloques.editor.layout";

const DEFAULTS = {
	objects: 190,
	right: 548,
	flyout: 280,
};

const LIMITS = {
	objects: [120, 420],
	right: [220, 900],
	flyout: [140, 720],
};

const WORKSPACE_MIN = 240;
const SPLITTER_PX = 4;

const listeners = new Set();
const sizes = loadSizes();

export function getFlyoutWidth() {
	return sizes.flyout;
}

export function onLayoutChange(fn) {
	listeners.add(fn);
	return () => listeners.delete(fn);
}

export function initSplitters() {
	clampToWindow();
	applySizes();
	document.querySelectorAll(".splitter[data-split]").forEach((el) => {
		el.addEventListener("pointerdown", (e) => onPointerDown(e, el));
	});
	window.addEventListener("resize", () => {
		clampToWindow();
		applySizes();
		notify("all");
	});
}

function loadSizes() {
	const next = {...DEFAULTS};
	try {
		const raw = localStorage.getItem(STORAGE_KEY);
		if (!raw) {
			return next;
		}
		const saved = JSON.parse(raw);
		for (const key of Object.keys(DEFAULTS)) {
			const n = Number(saved?.[key]);
			if (Number.isFinite(n)) {
				next[key] = clamp(n, LIMITS[key][0], LIMITS[key][1]);
			}
		}
	} catch {
		/* layout corrupto: usar defaults */
	}
	return next;
}

function saveSizes() {
	try {
		localStorage.setItem(STORAGE_KEY, JSON.stringify(sizes));
	} catch {
		/* storage lleno o bloqueado */
	}
}

function applySizes() {
	const root = document.documentElement;
	root.style.setProperty("--objects-width", `${sizes.objects}px`);
	root.style.setProperty("--right-width", `${sizes.right}px`);
	root.style.setProperty("--flyout-width", `${sizes.flyout}px`);
}

function notify(key) {
	for (const fn of listeners) {
		fn(key);
	}
}

function clamp(n, min, max) {
	return Math.min(max, Math.max(min, Math.round(n)));
}

function blocksWidth() {
	const el = document.querySelector(".blocks-panel");
	return el ? el.getBoundingClientRect().width : 108;
}

function maxFor(key) {
	const panels = document.querySelector(".panels");
	const total = panels?.clientWidth ?? window.innerWidth;
	const [absMin, absMax] = LIMITS[key];
	if (key === "flyout") {
		const host = document.querySelector(".workspace-body");
		const width = host?.clientWidth ?? 0;
		if (width < 80) {
			return absMax;
		}
		return clamp(width - 80, absMin, absMax);
	}
	const used = sizes.objects + sizes.right + blocksWidth() + SPLITTER_PX * 2;
	const room = total - WORKSPACE_MIN - (used - sizes[key]);
	return clamp(Math.max(room, absMin), absMin, absMax);
}

function clampToWindow() {
	sizes.objects = clamp(sizes.objects, LIMITS.objects[0], maxFor("objects"));
	sizes.right = clamp(sizes.right, LIMITS.right[0], maxFor("right"));
	const host = document.querySelector(".workspace-body");
	if (host && host.clientWidth >= 80) {
		sizes.flyout = clamp(sizes.flyout, LIMITS.flyout[0], maxFor("flyout"));
	}
}

function onPointerDown(e, el) {
	if (e.button !== 0) {
		return;
	}
	const key = el.dataset.split;
	if (!LIMITS[key]) {
		return;
	}
	e.preventDefault();
	el.classList.add("dragging");
	document.body.classList.add("col-resizing");
	el.setPointerCapture(e.pointerId);

	const startX = e.clientX;
	const start = sizes[key];
	const sign = key === "right" ? -1 : 1;
	const min = LIMITS[key][0];

	function move(ev) {
		const max = maxFor(key);
		sizes[key] = clamp(start + (ev.clientX - startX) * sign, min, max);
		applySizes();
		notify(key);
	}

	function stop(ev) {
		try {
			el.releasePointerCapture(ev.pointerId);
		} catch {
			/* ya liberado */
		}
		el.classList.remove("dragging");
		document.body.classList.remove("col-resizing");
		el.removeEventListener("pointermove", move);
		el.removeEventListener("pointerup", stop);
		el.removeEventListener("pointercancel", stop);
		saveSizes();
		notify(key);
	}

	el.addEventListener("pointermove", move);
	el.addEventListener("pointerup", stop);
	el.addEventListener("pointercancel", stop);
}
