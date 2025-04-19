const ESP_IP = '10.0.0.12';
let isMicActive = false;
let micStream;
let webSocket;
const micButton = document.getElementById('micStatusButton');

// Initialize WebSocket connection
function initWebSocket() {
    // Connect to the ESP32 WebSocket server (replace with actual ESP32 IP address)
    webSocket = new WebSocket(`ws://${ESP_IP}:81`);  // Replace with your ESP32 IP address

    webSocket.onopen = () => {
        console.log('WebSocket connected!');
    };

    webSocket.onclose = () => {
        console.log('WebSocket closed!');
    };

    webSocket.onerror = (error) => {
        console.error('WebSocket error:', error);
    };
}

// Toggle microphone status (mute/unmute)
function toggleMicStatus() {
    if (isMicActive) {
        stopMic();
    } else {
        startMic();
    }
}

// Start microphone and stream to WebSocket
async function startMic() {
    isMicActive = true;
    micButton.classList.remove('muted');
    micButton.classList.add('unmuted');
    micButton.textContent = 'Mic is On';

    // Initialize WebSocket if not already done
    if (!webSocket || webSocket.readyState !== WebSocket.OPEN) {
        initWebSocket();
    }

    // Request microphone access
    try {
        micStream = await navigator.mediaDevices.getUserMedia({ 
            audio: {
                echoCancellation: true,
                noiseSuppression: true,
                autoGainControl: true,
                sampleRate: 48000
            } 
        });
        const audioContext = new AudioContext({ sampleRate: 48000 });

        // Load the AudioWorklet
        await audioContext.audioWorklet.addModule('pcm-processor.js');

        const microphone = audioContext.createMediaStreamSource(micStream);
        const pcmProcessor = new AudioWorkletNode(audioContext, 'pcm-processor');

        microphone.connect(pcmProcessor);

        pcmProcessor.port.onmessage = (event) => {
            const pcmBuffer = event.data; // PCM data from the processor
            if (webSocket.readyState === WebSocket.OPEN) {
                webSocket.send(pcmBuffer); // Send PCM data as ArrayBuffer
            }
        };
    } catch (error) {
        console.error('Error accessing microphone:', error);
        micButton.classList.remove('unmuted');
        micButton.classList.add('muted');
        micButton.textContent = 'Mic Access Denied';
    }
}

// Stop microphone and stop sending data
function stopMic() {
    isMicActive = false;
    micButton.classList.remove('unmuted');
    micButton.classList.add('muted');
    micButton.textContent = 'Mic is Off';

    if (micStream) {
        const tracks = micStream.getTracks();
        tracks.forEach(track => track.stop());
    }
}