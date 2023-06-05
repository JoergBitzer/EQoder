# EQoder
a playable EQ

## Remarks:



## todo

* Switch parallel to seriel (at the moment). parallel seems more sensible. Done ==> Wrong idea, since the original is not coming thru or it will be always part of the signal for open filters.
* Allow Notch? Gain at the moment is always positive Done == > does not work 

Prio -2 AutoNormalisation = Apply negative Gain According to added Gain. Not sure if this is a good idea. Hard to explain.

Prio -4 Reprogram the core audio to AudioBuffer 

Done Prio 0 Build visualization tool for EQ Parameter.

Prio 0 Build on Mac and Windows (Done, tested with Cubase)

Produce presets and example files 

Produce 4 Videos
1 how to use (parameter explanation, simple example pink noise)

Drehbuch: 

1. Begrüßung
Hello, in this video I will show you how to use EQoder an its parameter. Eqoder is in its essence an amplyfiing comb filter playable with midi input.

2. Setting
For this introduction I will use the PluginHost provided with JUCE. EQoder is an insert 
effect plugin and needs an audio source. I will use Surge XT. Lets build a simple pink noise with
very long release phase. This will be our input signal for the start. 
Surge and EQoder have their own virtual keyboard. Lets use these as a start.

3. Parameter

First we have to define the number of FilterUnits, that is the same as the number of voices for a synthesizer. One is a good starting point to show the effect. Second we set the number of peak filters to the maximum of 20 and the gain to a constant of 10dB. High Q setting and Frequency spreading at 1 will guruantee a harmonic output. 

Start the input by playing a note with Surge and now I play a melody on Eqoder. 

Sound example 1 (Short Attack, Short release)

Its sounds like some hollow noisy sound. The envelope of the filter process can be adjusted with these four typical Envelope paramater.

Change to higher release and attack.

Lets now play with the parameters.

Frequency Spread changes the harmonicity. All Power of twos (0.5, 1, 2) will lead to harmonic amplification. 

change parameter ==> sound

BWSpread will change the bandwidth or Q factor versus frequency. As you can see, every change is displayed, so you see what you do.

This can be used for noisier sounds.

And finally we can change the number and form of our individual gains. 

more audio examples-


2 shows examples sound design (drum sounds, attack changer, long term modulation)h drum 

In this video I will show you one possible way to use Eqoder as a sound design tool. It acts like another filter module in your favourite synth. 

Lets start with some drum and percussion sounds. The source audio is pink noise from Surge XT. But this time we will use the envelope and adapt it to the desired sound.

we use the same midi keyboard as input.

Test some sounds


3 show examples music modulation (play a melody and play others on EQoder)

In this video I will show you another way to use Eqoder. For this application we need our audio workstation. For me  this is Cubase, but more or less all will work.

Lets start with a short, kind of slowly melody. The instrument needs to be rich in sound or a little noisy. Ok nice enoough. Lets record something.

And now we modulate this signal with another melody on top of its own sound with Eqoder.


4 how it works
The technical background (only for the nerds)



