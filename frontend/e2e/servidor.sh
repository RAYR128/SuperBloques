#!/usr/bin/env bash
# Playwright usa servidor.js (tambien en Windows). Este wrapper es para arrancar a mano.
cd "$(dirname "${BASH_SOURCE[0]}")"
exec node ./servidor.js
