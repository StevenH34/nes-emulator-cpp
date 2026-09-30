const canvas = document.getElementById('canvas');
const romInput = document.getElementById('rom-input');
const statusEl = document.getElementById('status');

function setStatus(text, isError = false) {
  statusEl.textContent = text;
  statusEl.classList.toggle('error', isError);
}

let nes;
try {
  // NesModule is the MODULARIZE factory defined by nes_emulator_wasm.js.
  nes = await NesModule({ canvas });
} catch (e) {
  setStatus(`Failed to start emulator: ${e}`, true);
  throw e;
}

romInput.disabled = false;
setStatus('Choose a .nes ROM to start.');

romInput.addEventListener('change', async () => {
  const file = romInput.files[0];
  if (!file) {
    return;
  }

  const bytes = new Uint8Array(await file.arrayBuffer());
  const error = nes.loadRom(bytes);
  if (error) {
    setStatus(`Could not load ${file.name}: ${error}`, true);
  } else {
    setStatus(`Running ${file.name}`);
  }
});
