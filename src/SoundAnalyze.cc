#include "cthugha.h"
#include "interface.h"
#include "display.h"
#include "cth_buffer.h"
#include "imath.h"
#include "CthughaBuffer.h"
#include "SoundAnalyze.h"
#include "CthughaDisplay.h"

#include <math.h>

OptionInt sound_minnoise ("minnoise",  5, 256);		/* quiet is below this */
OptionInt softwareGain ("software-gain", 10, 5, 50);	/* gain x0.1 (10=1.0x, 20=2.0x) */
OptionOnOff autoGain ("auto-gain", 0);			/* auto-gain normalization */

SoundAnalyze soundAnalyze;




SoundAnalyze::SoundAnalyze() {
    intensity = 0.0;
    currentGain = 1.0f;
}


void SoundAnalyze::operator()() {

    static double lastFires[16];
    static int lastFiresP = 0;

    static int lastamp = 0;
    int al = 0,ar = 0;

    /* Apply software gain to audio data */
    float gain = softwareGain / 10.0f;

    /* Auto-gain: target ~60% of full scale (76 out of 127) */
    if (autoGain) {
	float target = 76.0f;
	float peak = 1.0f;
	for (int ii = 0; ii < 1024; ii++) {
	    float lv = abs(soundDevice->data[ii][0]);
	    float rv = abs(soundDevice->data[ii][1]);
	    if (lv > peak) peak = lv;
	    if (rv > peak) peak = rv;
	}
	float desired = target / peak;
	/* Smooth: fast attack, slow release */
	if (desired < currentGain)
	    currentGain = currentGain * 0.7f + desired * 0.3f;
	else
	    currentGain = currentGain * 0.98f + desired * 0.02f;
	gain *= currentGain;
    }

    if (gain != 1.0f) {
	for (int ii = 0; ii < 1024; ii++) {
	    int lv = (int)(soundDevice->data[ii][0] * gain);
	    int rv = (int)(soundDevice->data[ii][1] * gain);
	    soundDevice->data[ii][0] = (char)(lv > 127 ? 127 : (lv < -128 ? -128 : lv));
	    soundDevice->data[ii][1] = (char)(rv > 127 ? 127 : (rv < -128 ? -128 : rv));
	}
    }

    /* get the amplitude of this sound frame (root mean squared) */
    char * d = (char*)soundDevice->data;
    for(int i=1024; i!=0; i--) {
	al += *d * *d; d++;
	ar += *d * *d; d++;
    }
    /* sqare root the mean */
    al = int(sqrt(double(al)/1024));
    ar = int(sqrt(double(ar)/1024));
    
    amplitude = (al+ar)/2;
    amplitudeLeft = al;
    amplitudeRight = ar;
    
    if (amplitude < lastamp-3)		/* faster decay for snappier response */
	amplitude = lastamp-3;
    
    if (amplitude > lastamp)
	attackLevel += amplitude-lastamp;
    
    if( (now - lastFires[lastFiresP]) > 0.001)
        speed = 16.0 / (now - lastFires[lastFiresP]);
    

    /* if the attack is finally over, then fire at the intensity of the attack */
    if (amplitude < lastamp) {
	fire = attackLevel;
	attackLevel = 0;

	lastFires[lastFiresP] = now;
	lastFiresP = (lastFiresP + 1) % 16;
    } else 
	fire = 0;
    fireLevel += fire;

    lastamp = amplitude;
    
    /* check for silence */
    noisy = ( (amplitudeLeft >= sound_minnoise) || (amplitudeRight >= sound_minnoise) );
    
    intensity = intensity * 0.85 + (amplitude / 128.0) * 0.15;

#if 0
    /* compute beats/minute, not working as it should */
    if(soundAnalyze->fire > 20) {
	static int bt[16];
	static int bn=0;
	int sound_bpm;

	bt[bn] = gettime();

	sound_bpm = bt[bn] - bt[ (bn+1)%16 ];		/* time for 16 beats */
	if(sound_bpm > 0) {
	    sound_bpm = 16*60000/sound_bpm;
	}
	printfv(5, "bpm: %d\n", sound_bpm);

	bn = (bn+1)%16;
    }
#endif
}





 	








