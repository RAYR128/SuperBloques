// TO-DO: interaccion con WASM
const title = document.getElementById("cat-title");
document.querySelectorAll(".cat").forEach((btn) => {
	btn.addEventListener("click", () => {
		document.querySelectorAll(".cat").forEach((b) => b.classList.remove("active"));
		btn.classList.add("active");
		title.textContent = btn.textContent.trim();
	});
});

document.querySelectorAll(".tree-item").forEach((item) => {
	item.addEventListener("click", () => {
		document.querySelectorAll(".tree-item").forEach((i) => i.classList.remove("active"));
		item.classList.add("active");
	});
});