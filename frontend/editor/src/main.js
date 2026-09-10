import {initMenu} from "./menu.js";
import {initTree} from "./tree.js";
import {initWorkspace} from "./blocks/workspace.js";

function info(html) {
	const el = document.querySelector(".debug-body");
	if (el) {
		el.innerHTML = html;
	}
}

window.addEventListener("error", (e) => {
	info(`<div class="warn">[error] ${e.message}</div><div class="dim">${e.filename}:${e.lineno}</div>`);
});

try {
	initWorkspace();
	initTree();
	initMenu();
	const params = new URLSearchParams(location.search);
	const cat = params.get("cat");
	if (cat) {
		document.querySelector(`.cat[data-cat="${cat}"]`)?.click();
	}
	if (params.has("menu")) {
		document.getElementById("menu-file")?.click();
	}
	info('<div class="ok">[ok] editor listo</div>');
} catch (e) {
	console.error(e);
	info(`<div class="warn">[error] ${e.message}</div><div class="dim">${e.stack ?? ""}</div>`);
}
