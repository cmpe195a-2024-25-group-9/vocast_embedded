let isMicActive = false;
        let micStream;
        let webSocket;
        const micButton = document.getElementById('micStatusButton');

        // Initialize WebSocket connection
        function initWebSocket() {
            // Connect to the ESP32 WebSocket server (replace with actual ESP32 IP address)
            webSocket = new WebSocket('ws://172.20.10.2:81');  // Replace with your ESP32 IP address
            
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
                const audioContext = new (window.AudioContext || window.webkitAudioContext)();
                const microphone = audioContext.createMediaStreamSource(micStream);
                const analyser = audioContext.createAnalyser();
                microphone.connect(analyser);
                
                const bufferLength = analyser.frequencyBinCount;
                const dataArray = new Uint8Array(bufferLength);
                
                function sendAudioData() {
                    if (isMicActive && webSocket.readyState === WebSocket.OPEN) {
                        analyser.getByteFrequencyData(dataArray);
                        webSocket.send(dataArray);  // Send audio data to ESP32 via WebSocket
                    }
                    requestAnimationFrame(sendAudioData);
                }

                sendAudioData();  // Start sending data
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