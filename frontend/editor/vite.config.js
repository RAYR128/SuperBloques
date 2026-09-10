import fs from "node:fs";
import path from "node:path";
import {fileURLToPath} from "node:url";
import {defineConfig} from "vite";

const root = path.dirname(fileURLToPath(import.meta.url));
const mediaSrc = path.join(root, "node_modules", "blockly", "media");

const MEDIA_TYPES = {
	".png": "image/png",
	".svg": "image/svg+xml",
	".mp3": "audio/mpeg",
	".wav": "audio/wav",
	".cur": "image/x-icon",
	".css": "text/css",
	".js": "text/javascript",
};

function blocklyMedia() {
	return {
		name: "blockly-media",
		configureServer(server) {
			server.middlewares.use((req, res, next) => {
				const url = (req.url ?? "").split("?")[0];
				const prefix = "/editor/media/";
				if (!url.startsWith(prefix)) {
					return next();
				}
				const rel = decodeURIComponent(url.slice(prefix.length));
				const file = path.resolve(mediaSrc, rel);
				if (!file.startsWith(mediaSrc + path.sep) && file !== mediaSrc) {
					res.statusCode = 403;
					res.end();
					return;
				}
				if (!fs.existsSync(file) || fs.statSync(file).isDirectory()) {
					res.statusCode = 404;
					res.end();
					return;
				}
				const ext = path.extname(file).toLowerCase();
				res.setHeader("Content-Type", MEDIA_TYPES[ext] || "application/octet-stream");
				fs.createReadStream(file).pipe(res);
			});
		},
		closeBundle() {
			const dest = path.join(root, "dist", "media");
			fs.cpSync(mediaSrc, dest, {recursive: true});
		},
	};
}

export default defineConfig({
	base: "/editor/",
	build: {
		outDir: "dist",
		emptyOutDir: true,
		assetsDir: "assets",
	},
	server: {
		port: 5173,
	},
	plugins: [blocklyMedia()],
});
