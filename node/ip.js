#!/usr/bin/env node

console.log(
  Object.values(require("os").networkInterfaces())
    .flat()
    .find(({ family, internal }) => !internal && family === "IPv4")?.address,
);
