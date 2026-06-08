// -*- c++ -*-

#ifndef __BEAT_DETECT_H
#define __BEAT_DETECT_H

#include "CoreOption.h"

/*
 * Frequency-band beat detection.
 *
 * Splits audio into 3 bands (bass, mid, treble) using a simple DFT
 * energy computation, then detects beats per-band using a ratio of
 * instantaneous energy to a short history average.
 */

#define BEAT_HISTORY 32

enum BeatBand {
    BEAT_BASS = 0,
    BEAT_MID = 1,
    BEAT_TREBLE = 2,
    BEAT_NUM_BANDS = 3
};

class BeatDetect {
public:
    BeatDetect();

    void analyze();		// call once per audio frame

    // Per-band state
    float energy[BEAT_NUM_BANDS];	// current energy
    float average[BEAT_NUM_BANDS];	// running average energy
    float ratio[BEAT_NUM_BANDS];	// energy / average (>1 = beat)
    int   beat[BEAT_NUM_BANDS];		// 1 if beat detected this frame

    // Overall
    int   anyBeat;			// any band fired
    float totalEnergy;			// broadband energy
    float sensitivity;			// beat threshold multiplier (default 1.4)

private:
    float history[BEAT_NUM_BANDS][BEAT_HISTORY];
    int   histPos;
};

extern BeatDetect beatDetect;
extern OptionInt beatSensitivity;

#endif
