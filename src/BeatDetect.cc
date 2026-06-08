#include "cthugha.h"
#include "BeatDetect.h"
#include "SoundDevice.h"

#include <math.h>
#include <string.h>

/*
 * Beat sensitivity: 10 = threshold of 1.0 (everything is a beat),
 * 20 = threshold of 2.0 (need 2x average energy). Default 14 = 1.4x.
 */
OptionInt beatSensitivity("beat-sensitivity", 14, 10, 30);

BeatDetect beatDetect;

BeatDetect::BeatDetect() {
    memset(history, 0, sizeof(history));
    memset(energy, 0, sizeof(energy));
    memset(average, 0, sizeof(average));
    memset(ratio, 0, sizeof(ratio));
    memset(beat, 0, sizeof(beat));
    histPos = 0;
    anyBeat = 0;
    totalEnergy = 0;
    sensitivity = 1.4f;
}

void BeatDetect::analyze() {
    /*
     * Compute energy in 3 frequency bands from raw 8-bit audio.
     * We use the magnitude of the time-domain signal in overlapping
     * "pseudo-bands" derived from simple windowed energy:
     *
     * Bass:   samples 0-255   (~0-2kHz at 48kHz, dominated by low freq)
     * Mid:    samples 256-639 (~2-6kHz)
     * Treble: samples 640-1023 (~6kHz+)
     *
     * This is crude but fast — a proper approach would use FFT bins,
     * but we want minimal latency and the existing FFT is optional.
     */

    static const int bandStart[BEAT_NUM_BANDS] = { 0, 256, 640 };
    static const int bandEnd[BEAT_NUM_BANDS]   = { 256, 640, 1024 };

    sensitivity = beatSensitivity / 10.0f;
    totalEnergy = 0;

    for (int band = 0; band < BEAT_NUM_BANDS; band++) {
        float sum = 0;
        int count = bandEnd[band] - bandStart[band];

        for (int ii = bandStart[band]; ii < bandEnd[band]; ii++) {
            float left = soundDevice->data[ii][0];
            float right = soundDevice->data[ii][1];
            sum += left * left + right * right;
        }
        energy[band] = sum / count;
        totalEnergy += energy[band];

        /* Update running average */
        history[band][histPos] = energy[band];
        float avg = 0;
        for (int jj = 0; jj < BEAT_HISTORY; jj++)
            avg += history[band][jj];
        average[band] = avg / BEAT_HISTORY;

        /* Detect beat: instantaneous energy exceeds average by threshold */
        if (average[band] > 0.001f) {
            ratio[band] = energy[band] / average[band];
            beat[band] = (ratio[band] > sensitivity) ? 1 : 0;
        } else {
            ratio[band] = 0;
            beat[band] = 0;
        }
    }

    histPos = (histPos + 1) % BEAT_HISTORY;
    anyBeat = beat[BEAT_BASS] || beat[BEAT_MID] || beat[BEAT_TREBLE];
}
