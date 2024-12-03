class MicProcessor extends AudioWorkletProcessor {
    process(inputs, outputs, parameters) {
        const input = inputs[0];
        if (input && input[0]) {
            const rawData = input[0]; // Float32Array from input channel
            const int16Buffer = new Int16Array(rawData.length);

            for (let i = 0; i < rawData.length; i++) {
                int16Buffer[i] = rawData[i] * 0x7FFF; // Scale Float32 to Int16
            }

            this.port.postMessage(int16Buffer.buffer); // Send PCM data
        }

        return true; // Keep processor alive
    }
}

registerProcessor('mic-processor', MicProcessor);