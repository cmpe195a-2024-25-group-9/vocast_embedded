let isMicActive = false;
let micStream;
let webSocket;
const micButton = document.getElementById('micStatusButton');

/*   old snippet from live streaming audio on i2s on esp
      if (length % sizeof(int16_t) == 0) {
        if (!payload || length == 0 || length > 1024) {  // Ensure payload is valid
          Serial.println("Invalid payload received!");
          return;
        }
        memcpy(audioBuffer, payload, min(length, sizeof(audioBuffer))); // payload, length
        size_t bytesWritten = 0; // put in directly instead of into auido buffer 
        esp_err_t err = i2s_write(I2S_NUM_0, payload, length, &bytesWritten, 10 / portTICK_PERIOD_MS); // pass into audio buffer instead of payload

        if (err != ESP_OK) {
            Serial.println(F("I2S write failed!"));
        } 
        // free(audioBuffer);
        memset(audioBuffer, 0, sizeof(audioBuffer));
      } */

// Initialize WebSocket connection
function initWebSocket() {
    // Connect to the ESP32 WebSocket server (replace with actual ESP32 IP address)
    webSocket = new WebSocket('ws://172.20.10.3:81');  // Replace with your ESP32 IP address
    
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
        const audioContext = new AudioContext({ sampleRate: 48000 });

        // Load the AudioWorklet
        await audioContext.audioWorklet.addModule('pcm-processor.js');

        const microphone = audioContext.createMediaStreamSource(micStream);
        const pcmProcessor = new AudioWorkletNode(audioContext, 'pcm-processor');

        microphone.connect(pcmProcessor);

        pcmProcessor.port.onmessage = (event) => {
            const pcmBuffer = event.data; // PCM data from the processor
            if (webSocket.readyState === WebSocket.OPEN) {
                // console log START time
                // if (i > 0) {
                    // console.log("send time:");
                    // console.log(new Date().getTime());
                    webSocket.send(pcmBuffer); // Send PCM data as ArrayBuffer
                    // i--;
                // }
            }
        }; 
    } catch (error) {
        console.error('Error accessing microphone:', error);
        micButton.classList.remove('unmuted');
        micButton.classList.add('muted');
        micButton.textContent = 'Mic Access Denied';
    }

    /* 
    webSocket.onmessage = (event) => {
        /* const response = JSON.parse(event.data);
        const clientSendTime = response.timestamp; // The original timestamp
        const clientReceiveTime = Date.getTime();      // Time of echo receipt
    
        const rtt = clientReceiveTime - clientSendTime; // Round-trip time
        const oneWayDelay = rtt / 2;                    // Approximate one-way delay

        console.log("receive time:");
        console.log(new Date().getTime());
    }; */
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