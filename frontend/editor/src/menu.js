import {exportarProyecto} from "./export.js";
import {importarProyecto} from "./import.js";
import {flushWorkspace} from "./blocks/workspace.js";

export function initMenu() {
	const wrap = document.getElementById("menu-file-wrap");
	const btn = document.getElementById("menu-file");
	const dropdown = document.getElementById("menu-file-dropdown");
	const importBtn = document.getElementById("menu-import");
	const exportBtn = document.getElementById("menu-export");

	function cerrar() {
		dropdown.hidden = true;
		wrap.classList.remove("open");
	}

	function abrir() {
		dropdown.hidden = false;
		wrap.classList.add("open");
	}

	btn.addEventListener("click", (e) => {
		e.stopPropagation();
		if (dropdown.hidden) {
			abrir();
		} else {
			cerrar();
		}
	});

	importBtn.addEventListener("click", () => {
		cerrar();
		importarProyecto();
	});

	exportBtn.addEventListener("click", () => {
		flushWorkspace();
		exportarProyecto();
		cerrar();
	});

	document.addEventListener("click", (e) => {
		if (!wrap.contains(e.target)) {
			cerrar();
		}
	});

	document.addEventListener("keydown", (e) => {
		if (e.key === "Escape") {
			cerrar();
		}
	});
}
