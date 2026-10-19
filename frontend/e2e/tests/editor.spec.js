import fs from "node:fs/promises";
import os from "node:os";
import path from "node:path";
import {expect, test} from "@playwright/test";
import {registrar} from "../helpers/cuenta.js";

async function abrirEditor(page) {
	await page.goto("/editor/");
	await expect(page.locator(".debug-body")).toContainText("[ok] editor listo");
}

async function abrirMenu(page) {
	await page.locator("#menu-file").click();
	await expect(page.locator("#menu-file-dropdown")).toBeVisible();
}

function alDialogo(page, responder) {
	return new Promise((resolve, reject) => {
		page.once("dialog", (dialog) => {
			responder(dialog).then(resolve, reject);
		});
	});
}

async function aceptarPrompt(page, accion, valor) {
	let tipo = "";
	const listo = alDialogo(page, (dialog) => {
		tipo = dialog.type();
		return tipo === "prompt" ? dialog.accept(valor) : dialog.dismiss();
	});
	await accion();
	await listo;
	expect(tipo).toBe("prompt");
}

async function promptYAlerta(page, accion, valor) {
	const mensajes = [];
	let pendientes = 2;
	const listo = new Promise((resolve, reject) => {
		const handler = (dialog) => {
			mensajes.push({tipo: dialog.type(), mensaje: dialog.message()});
			const respuesta = dialog.type() === "prompt" ? dialog.accept(valor) : dialog.accept();
			respuesta.then(() => {
				pendientes -= 1;
				if (pendientes === 0) {
					page.off("dialog", handler);
					resolve(mensajes);
				}
			}, reject);
		};
		page.on("dialog", handler);
	});
	await accion();
	return listo;
}

test("el editor anonimo abre un proyecto nuevo", async ({page}) => {
	await abrirEditor(page);
	await expect(page.locator("header .sub")).toHaveText("Nuevo Proyecto");
	await expect(page.locator("#header-user")).toContainText("Entrar");
	await expect(page.locator('[data-tipo="escena"][data-nombre="Escena1"]')).toBeVisible();
	await expect(page.locator("#stage")).toHaveAttribute("width", "512");
	await expect(page.locator("#stage")).toHaveAttribute("height", "448");
	await expect(page.locator("#blocklyDiv .blocklySvg")).toBeVisible();
	await expect(page.locator('.cat[data-cat="mocion"]')).toBeDisabled();
	await expect(page.locator('.cat[data-cat="animacion"]')).toHaveClass(/active/);
});

test("el menu Archivo abre y Escape lo cierra", async ({page}) => {
	await abrirEditor(page);
	await abrirMenu(page);
	await expect(page.locator("#menu-import")).toBeVisible();
	await expect(page.locator("#menu-export")).toBeVisible();
	await expect(page.locator("#menu-rom")).toBeVisible();
	await expect(page.locator("#menu-publish")).toBeVisible();
	await page.keyboard.press("Escape");
	await expect(page.locator("#menu-file-dropdown")).toBeHidden();
});

test("la categoria Control queda activa", async ({page}) => {
	await abrirEditor(page);
	await expect(page.locator('.cat[data-cat="animacion"]')).toHaveClass(/active/);
	await page.locator('.cat[data-cat="control"]').click();
	await expect(page.locator('.cat[data-cat="control"]')).toHaveClass(/active/);
	await expect(page.locator('.cat[data-cat="animacion"]')).not.toHaveClass(/active/);
});

test("una escena nueva entra al arbol", async ({page}) => {
	await abrirEditor(page);
	await aceptarPrompt(page, () => page.locator("#add-escena").click(), "Bosque");
	const item = page.locator('[data-tipo="escena"][data-nombre="Bosque"]');
	await expect(item).toBeVisible();
	await expect(item).toHaveClass(/active/);
});

test("un nombre vacio no crea la escena", async ({page}) => {
	await abrirEditor(page);
	const mensajes = await promptYAlerta(page, () => page.locator("#add-escena").click(), "   ");
	expect(mensajes.map((d) => d.mensaje)).toEqual([
		"Nombre de la escena",
		"El nombre no puede estar vacio",
	]);
	await expect(page.locator("#tree-escenas .tree-item")).toHaveCount(1);
	await expect(page.locator('[data-nombre="Escena1"]')).toBeVisible();
});

test("un nombre repetido no crea la escena", async ({page}) => {
	await abrirEditor(page);
	const mensajes = await promptYAlerta(page, () => page.locator("#add-escena").click(), "Escena1");
	expect(mensajes.map((d) => d.mensaje)).toEqual([
		"Nombre de la escena",
		"Ya existe un elemento con ese nombre",
	]);
	await expect(page.locator("#tree-escenas .tree-item")).toHaveCount(1);
});

test("un objeto nuevo entra al arbol", async ({page}) => {
	await abrirEditor(page);
	await aceptarPrompt(page, () => page.locator("#add-objeto").click(), "Moneda");
	const item = page.locator('[data-tipo="objeto"][data-nombre="Moneda"]');
	await expect(item).toBeVisible();
	await expect(item).toHaveClass(/active/);
});

test("eliminar una escena la saca del arbol", async ({page}) => {
	await abrirEditor(page);
	await aceptarPrompt(page, () => page.locator("#add-escena").click(), "Bosque");
	let confirmacion = "";
	const confirmListo = alDialogo(page, (dialog) => {
		confirmacion = dialog.message();
		return dialog.accept();
	});
	await page.locator('[data-nombre="Bosque"] .tree-remove').click();
	await confirmListo;
	expect(confirmacion).toBe('Eliminar escena "Bosque"?');
	await expect(page.locator('[data-nombre="Bosque"]')).toHaveCount(0);
	await expect(page.locator('[data-nombre="Escena1"]')).toBeVisible();
});

test("exportar descarga el JSON del proyecto", async ({page}) => {
	await abrirEditor(page);
	await abrirMenu(page);
	const descargaLista = page.waitForEvent("download");
	await page.locator("#menu-export").click();
	const descarga = await descargaLista;
	expect(descarga.suggestedFilename()).toBe("Nuevo Proyecto.json");

	const archivo = path.join(os.tmpdir(), `sb-export-${Date.now()}.json`);
	await descarga.saveAs(archivo);
	const json = JSON.parse(await fs.readFile(archivo, "utf8"));
	expect(json.Escenas).toHaveProperty("Escena1");
	expect(json.Objetos).toEqual({});
});

test("importar restaura el proyecto exportado", async ({page}) => {
	await abrirEditor(page);
	await abrirMenu(page);
	const descargaLista = page.waitForEvent("download");
	await page.locator("#menu-export").click();
	const descarga = await descargaLista;
	const archivo = path.join(os.tmpdir(), `sb-roundtrip-${Date.now()}.json`);
	await descarga.saveAs(archivo);
	const contenido = await fs.readFile(archivo);

	await aceptarPrompt(page, () => page.locator("#add-escena").click(), "Bosque");
	await expect(page.locator('[data-nombre="Bosque"]')).toBeVisible();

	await abrirMenu(page);
	const chooserListo = page.waitForEvent("filechooser");
	await page.locator("#menu-import").click();
	const chooser = await chooserListo;
	await chooser.setFiles({
		name: "Nuevo Proyecto.json",
		mimeType: "application/json",
		buffer: contenido,
	});

	await expect(page.locator(".debug-body")).toContainText("proyecto importado");
	await expect(page.locator("header .sub")).toHaveText("Nuevo Proyecto");
	await expect(page.locator('[data-nombre="Bosque"]')).toHaveCount(0);
	await expect(page.locator('[data-nombre="Escena1"]')).toBeVisible();
});

test("un JSON invalido no reemplaza el proyecto", async ({page}) => {
	await abrirEditor(page);
	await abrirMenu(page);
	const chooserListo = page.waitForEvent("filechooser");
	let mensaje = "";
	const alertaLista = alDialogo(page, (dialog) => {
		mensaje = dialog.message();
		return dialog.accept();
	});
	await page.locator("#menu-import").click();
	const chooser = await chooserListo;
	await chooser.setFiles({
		name: "malo.json",
		mimeType: "application/json",
		buffer: Buffer.from("{"),
	});
	await alertaLista;
	expect(mensaje).toContain("No se pudo importar:");
	await expect(page.locator('[data-nombre="Escena1"]')).toBeVisible();
	await expect(page.locator(".debug-body")).toContainText("el proyecto en memoria no se modifico");
});

test("publicar sin sesion lleva al login", async ({page}) => {
	await abrirEditor(page);
	await abrirMenu(page);
	await page.locator("#menu-publish").click();
	await expect(page).toHaveURL(/\/login\?next=/);
	expect(new URL(page.url()).searchParams.get("next")).toBe("/editor/");
});

test("publicar con sesion muestra el proyecto en el sitio", async ({page}) => {
	const user = await registrar(page, {display: "Autor PW"});
	const nombre = `Juego ${Date.now().toString(36)}`;

	await abrirEditor(page);
	await expect(page.locator("#header-user")).toContainText("Autor PW");
	await abrirMenu(page);
	await page.locator("#menu-publish").click();
	await expect(page.locator("#publish-title")).toHaveText("Publicar proyecto");

	await page.locator("#publish-name").fill("");
	await page.locator("#publish-submit").click();
	await expect(page.locator("#publish-error")).toHaveText("El nombre no puede estar vacio");

	await page.locator("#publish-name").fill(nombre);
	await page.locator("#publish-submit").click();
	await expect(page.locator(".debug-body")).toContainText("proyecto publicado");
	await expect.poll(() => page.evaluate(() => location.pathname + location.search)).toMatch(/^\/editor\/\?project=\d+$/);
	await expect(page.locator("header .sub")).toHaveText(nombre);

	const id = await page.evaluate(() => new URLSearchParams(location.search).get("project"));
	await abrirMenu(page);
	await page.locator("#menu-publish").click();
	await expect(page.locator("#publish-title")).toHaveText("Actualizar proyecto");
	await expect(page.locator("#publish-submit")).toHaveText("Actualizar");
	await page.locator("#publish-cancel").click();
	await expect(page.locator("#publish-modal")).toBeHidden();

	await page.goto("/");
	await expect(page.locator("a.card-title", {hasText: nombre})).toBeVisible();

	await page.goto(`/project/${id}`);
	await expect(page.locator(".debug-body")).toContainText(nombre);

	await page.goto(`/u/${user.id}`);
	await expect(page.locator("a.card-title", {hasText: nombre})).toBeVisible();
});
