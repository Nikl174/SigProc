async function start() {
  const wasm = await WebAssembly.instantiateStreaming(
    fetch("./main.wasm"),
    // provide functions that can be 'imported' into WebAssembly
    {
      "js": {
        "print": (x) => console.log(`A WebAssembly function call with argument ${x}, yay!`)
      }
    },
  );
  console.log(wasm.instance.exports.add(22, 20));
  // console.log(wasm);
}

start().catch((e) => console.error(e));
