class PCMProcessor extends AudioWorkletProcessor {
    process(inputs, outputs, parameters) {
        const input = inputs[0];
        if (input.length > 0) {
            const leftChannel = input[0];  // Left channel
            const rightChannel = input[1] || leftChannel;  // Right channel (or mirror left if not provided)
            const pcmData = new Int16Array(leftChannel.length * 2); // Stereo interleaved buffer
    
            for (let i = 0; i < leftChannel.length; i++) {
                pcmData[i * 2] = Math.max(-1, Math.min(1, leftChannel[i])) * 32767;  // Left
                pcmData[i * 2 + 1] = Math.max(-1, Math.min(1, rightChannel[i])) * 32767;  // Right
            }
    
            this.port.postMessage(pcmData.buffer);
        }
        return true;
    }
}

registerProcessor('pcm-processor', PCMProcessor);
