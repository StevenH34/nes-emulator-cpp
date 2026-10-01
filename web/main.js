const canvas = document.getElementById('canvas');
const romInput = document.getElementById('rom-input');
const statusEl = document.getElementById('status');

function setStatus(text, isError = false) {
    statusEl.textContent = text;
    statusEl.classList.toggle('error', isError);
}

let nes;

async function loadRomFile(file) {
    const bytes = new Uint8Array(await file.arrayBuffer());
    const error = nes.loadRom(bytes);
    if (error) {
        setStatus(`Could not load ${file.name}: ${error}`, true);
    } else {
        setStatus(`Running ${file.name}`);
    }
}

// The whole page is a drop target.
const isFileDrag = (event) => event.dataTransfer?.types.includes('Files');
let dragDepth = 0; // dragenter/dragleave also fire when crossing child elements

window.addEventListener('dragenter', (event) => {
    if (!isFileDrag(event)) {
        return;
    }
    event.preventDefault();
    event.stopPropagation();
    dragDepth++;
    document.body.classList.toggle('dragging', nes !== undefined);
}, true);

window.addEventListener('dragover', (event) => {
    if (!isFileDrag(event)) {
        return;
    }
    event.preventDefault();
    event.stopPropagation();
    event.dataTransfer.dropEffect = nes !== undefined ? 'copy' : 'none';
}, true);

window.addEventListener('dragleave', (event) => {
    if (!isFileDrag(event)) {
        return;
    }
    event.stopPropagation();
    dragDepth = Math.max(0, dragDepth - 1);
    if (dragDepth === 0) {
        document.body.classList.remove('dragging');
    }
}, true);

window.addEventListener('drop', (event) => {
    if (!isFileDrag(event)) {
        return;
    }
    event.preventDefault();
    event.stopPropagation();
    dragDepth = 0;
    document.body.classList.remove('dragging');

    const file = event.dataTransfer.files[0];
    if (nes !== undefined && file) {
        loadRomFile(file);
    }
}, true);

try {
    // NesModule is the MODULARIZE factory defined by nes_emulator_wasm.js.
    nes = await NesModule({canvas});
} catch (e) {
    setStatus(`Failed to start emulator: ${e}`, true);
    throw e;
}

romInput.disabled = false;
setStatus('Choose or drop a .nes ROM to start.');

romInput.addEventListener('change', () => {
    const file = romInput.files[0];
    if (file) {
        loadRomFile(file);
    }
});
