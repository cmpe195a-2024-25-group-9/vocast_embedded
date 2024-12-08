let isMicActive = false;
let micStream;
let webSocket;
let i = 10;
const micButton = document.getElementById('micStatusButton');

// Initialize WebSocket connection
function initWebSocket() {
    // Connect to the ESP32 WebSocket server (replace with actual ESP32 IP address)
    webSocket = new WebSocket('ws://10.0.0.222:81');  // Replace with your ESP32 IP address
    
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
        micStream = await navigator.mediaDevices.getUserMedia({ audio: true });
        const audioContext = new AudioContext();

        // Load the AudioWorklet
        await audioContext.audioWorklet.addModule('pcm-processor.js');

        const microphone = audioContext.createMediaStreamSource(micStream);
        const pcmProcessor = new AudioWorkletNode(audioContext, 'pcm-processor');

        microphone.connect(pcmProcessor);

        pcmProcessor.port.onmessage = (event) => {
            const pcmBuffer = event.data; // PCM data from the processor
            if (webSocket.readyState === WebSocket.OPEN) {
                // console log START time
                if (i > 0) {
                    console.log("send time:");
                    console.log(new Date().getTime());
                    webSocket.send(pcmBuffer); // Send PCM data as ArrayBuffer
                    i--;
                }
            }
        };
    } catch (error) {
        console.error('Error accessing microphone:', error);
        micButton.classList.remove('unmuted');
        micButton.classList.add('muted');
        micButton.textContent = 'Mic Access Denied';
    }

    webSocket.onmessage = (event) => {
        /* const response = JSON.parse(event.data);
        const clientSendTime = response.timestamp; // The original timestamp
        const clientReceiveTime = Date.getTime();      // Time of echo receipt
    
        const rtt = clientReceiveTime - clientSendTime; // Round-trip time
        const oneWayDelay = rtt / 2;                    // Approximate one-way delay
        */

        console.log("receive time:");
        console.log(new Date().getTime());
    };
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