import {expect, test} from "@playwright/test";
import {PASSWORD, nombreUnico, registrar} from "../helpers/cuenta.js";

test.describe.configure({mode: "serial"});

test("la galeria empieza vacia para un visitante anonimo", async ({page}) => {
	await page.goto("/");
	await expect(page.getByRole("heading", {name: "Proyectos recientes"})).toBeVisible();
	await expect(page.locator("#gallery")).toContainText("Todavia no hay proyectos publicados.");
	await expect(page.getByRole("link", {name: "Entrar", exact: true})).toBeVisible();
	await expect(page.getByRole("link", {name: "Crear cuenta", exact: true})).toBeVisible();
	await expect(page.getByRole("link", {name: "Editor", exact: true})).toBeVisible();
	await expect(page.getByRole("link", {name: "Abrir editor"})).toBeVisible();
});

test("un registro valido entra y muestra el nombre visible", async ({page}) => {
	const nombre = nombreUnico();
	await page.goto("/register");
	await page.locator('input[name="nombre"]').fill(nombre);
	await page.locator('input[name="display"]').fill("Nuevo Visible");
	await page.locator('input[name="password"]').fill(PASSWORD);
	await page.getByRole("button", {name: "Registrarse"}).click();

	await expect(page).toHaveURL("/");
	await expect(page.locator("#nav a[href^='/u/']")).toHaveText("Nuevo Visible");
	await expect(page.getByRole("link", {name: "Cuenta"})).toBeVisible();
	await expect(page.getByRole("button", {name: "Salir"})).toBeVisible();
});

test("un usuario duplicado muestra el error y se queda en el registro", async ({page, request}) => {
	const nombre = nombreUnico();
	await registrar(request, {nombre, display: "Ya Existe"});

	await page.goto("/register");
	await page.locator('input[name="nombre"]').fill(nombre);
	await page.locator('input[name="display"]').fill("Otro Nombre");
	await page.locator('input[name="password"]').fill(PASSWORD);
	await page.getByRole("button", {name: "Registrarse"}).click();

	await expect(page.locator("#auth-error")).toHaveText("ese usuario ya existe");
	await expect(page).toHaveURL("/register");
});

test("el login rechaza una clave mala y acepta la buena", async ({page, request}) => {
	const user = await registrar(request, {display: "Entra Ya"});

	await page.goto("/login");
	await page.locator('input[name="nombre"]').fill(user.nombre);
	await page.locator('input[name="password"]').fill("clave-mala");
	await page.getByRole("button", {name: "Entrar"}).click();
	await expect(page.locator("#auth-error")).toHaveText("usuario o contraseña incorrectos");
	await expect(page).toHaveURL("/login");

	await page.locator('input[name="password"]').fill(PASSWORD);
	await page.getByRole("button", {name: "Entrar"}).click();
	await expect(page).toHaveURL("/");
	await expect(page.locator("#nav a[href^='/u/']")).toHaveText("Entra Ya");

	// El primer GET /login queda en la cache del navegador. Sin esto, el segundo
	// no llega al servidor y no se aplica la redireccion de quien ya entro.
	await page.route("**/login", (route) => route.continue());
	await page.goto("/login");
	await expect(page).toHaveURL("/");
});

test("salir vuelve al nav anonimo", async ({page}) => {
	await registrar(page, {display: "Sale Ya"});
	await page.goto("/");
	await expect(page.locator("#nav")).toContainText("Sale Ya");

	await page.getByRole("button", {name: "Salir"}).click();
	await expect(page).toHaveURL("/");
	await expect(page.getByRole("link", {name: "Entrar"})).toBeVisible();
	await expect(page.getByRole("link", {name: "Crear cuenta"})).toBeVisible();
	await expect(page.getByRole("button", {name: "Salir"})).toHaveCount(0);
});

test("la cuenta sin sesion redirige al login", async ({page}) => {
	await page.goto("/account");
	await expect(page).toHaveURL("/login?next=/account");
});

test("la cuenta guarda el nombre visible", async ({page}) => {
	const user = await registrar(page, {display: "Visible Uno"});
	await page.goto("/account");
	await expect(page.locator("#username")).toHaveText(user.nombre);
	await expect(page.locator("#display")).toHaveValue("Visible Uno");

	await page.locator("#display").fill("Visible Dos");
	await page.locator('input[name="passwordActual"]').fill(PASSWORD);
	await page.getByRole("button", {name: "Guardar"}).click();

	await expect(page.locator("#account-ok")).toHaveText("Cambios guardados");
	await expect(page.locator("#nav a[href^='/u/']")).toHaveText("Visible Dos");
});

test("la cuenta rechaza la contraseña actual", async ({page}) => {
	await registrar(page, {display: "Clave Mala"});
	await page.goto("/account");
	await page.locator('input[name="passwordActual"]').fill("clave-mala");
	await page.getByRole("button", {name: "Guardar"}).click();
	await expect(page.locator("#account-error")).toHaveText("contraseña actual incorrecta");
});

test("un proyecto inexistente muestra la pagina 404", async ({page}) => {
	const res = await page.goto("/project/999999");
	expect(res.status()).toBe(404);
	await expect(page.getByRole("heading", {name: "No encontrado"})).toBeVisible();
});

test("el perfil sin proyectos lo dice", async ({page}) => {
	const user = await registrar(page, {display: "Perfil Vacio"});
	await page.goto(`/u/${user.id}`);
	await expect(page.locator("#profile-head")).toContainText(`@${user.nombre}`);
	await expect(page.locator("#gallery")).toContainText("Este usuario todavia no publico proyectos.");
});
