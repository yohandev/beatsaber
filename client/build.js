const opt = {
    entryPoints: ['src/index.js'],
    outdir: 'pkg',

    bundle: true,
    minify: true,
    sourcemap: true,
};
require('esbuild').build(opt);