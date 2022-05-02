const opt = {
    entryPoints: ['src/index.js'],
    outdir: 'pkg',

    bundle: true,
    minify: true,
    sourcemap: true,
    loader: {
        '.obj': 'text',
    }
};
require('esbuild').build(opt);