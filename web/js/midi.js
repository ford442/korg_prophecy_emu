// Web MIDI API Support
let midiAccess;

async function initMidi() {
    if (!navigator.requestMIDIAccess) {
        console.log('Web MIDI API not supported');
        return;
    }
    
    try {
        midiAccess = await navigator.requestMIDIAccess();
        const inputs = midiAccess.inputs.values();
        const select = document.getElementById('midiInput');
        select.innerHTML = '';
        
        for (let input of inputs) {
            const option = document.createElement('option');
            option.value = input.id;
            option.text = input.name;
            select.appendChild(option);
            input.onmidimessage = handleMidiMessage;
        }
        
        if (select.options.length === 0) {
            select.innerHTML = '<option>No devices</option>';
        }
        
        // Listen for device changes
        midiAccess.onstatechange = () => {
            console.log('MIDI state changed');
            initMidi();
        };
    } catch (e) {
        console.log('MIDI access denied or unavailable:', e);
    }
}

function handleMidiMessage(msg) {
    const [status, data1, data2] = msg.data;
    const cmd = status >> 4;
    const channel = status & 0xf;
    
    if (cmd === 9 && data2 > 0) { // Note on
        playNote(data1);
    } else if (cmd === 8 || (cmd === 9 && data2 === 0)) { // Note off
        stopNote(data1);
    } else if (cmd === 0xB) { // CC
        // Could map to Module._handleControlChange if exposed
        console.log('CC', data1, data2);
    }
}
