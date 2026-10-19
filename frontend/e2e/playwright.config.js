import {defineConfig} from "@playwright/test";

export default defineConfig({
	testDir: "./tests",
	fullyParallel: false,
	workers: 1,
	forbidOnly: !!process.env.CI,
	retries: process.env.CI ? 1 : 0,
	expect: {timeout: 10_000},
	use: {
		baseURL: "http://127.0.0.1:8099",
		viewport: {width: 1440, height: 900},
		trace: "on-first-retry",
		screenshot: "only-on-failure",
	},
	webServer: {
		command: "bash ./servidor.sh",
		url: "http://127.0.0.1:8099/api/health",
		timeout: 120_000,
		reuseExistingServer: false,
		stdout: "ignore",
		stderr: "pipe",
	},
	projects: [
		{name: "sitio", testMatch: /sitio\.spec\.js/},
		{
			name: "editor",
			testMatch: /editor\.spec\.js/,
			dependencies: ["sitio"],
		},
	],
});
