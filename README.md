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
For this introduction I will use the PluginHost provided by JUCE. EQoder is an insert 
effect plugin and needs an audio source. We will use Surge. Lets build a simple pink noise with a
very long release phase. This will be our input signal for this video. 
Surge and EQoder have their own virtual keyboard. Lets use these for the beginning.

3. Parameter
Eqoder has several parameters and I will explain their usage first.

First we have to define the number of FilterUnits, that is equivalent to the number of voices for a synthesizer. One is a good starting point to show the effect. Second we set the number of peak filters to the maximum of 20 and the gain to a constant of 10dB. High Q setting and Frequency spreading at one will guarantee a harmonic output. 

We start the input signal by playing a note with Surge and we hear noise only and now I can play a melody with Eqoder. 

[ Sound example 1 (Short Attack, Short release) ]

This sounds like some hollow noisy sound. The envelope of the filter process can be adjusted with these four typical Envelope parameters. With softer attack and release times, we should increase the number of filter units. 

[ Change to higher release and attack  ]

We can play chords as well now.

[Play some chords]

Lets see what the other parameters do.

Frequency Spread changes the harmonicity. All Power of twos (0.5, 1, 2) will lead to harmonic amplification. 
Listen to the different color of the sound.

change parameter ==> sound

BWSpread will change the bandwidth or Q factor versus frequency. With higher values the higher filter become less steep.
As you can see, every change is displayed, so you see what you do. However, this is not the actual transfer function.

Higher settings can be used for noisier sounds.

And finally we can change the number and form of our individual gains. 

[ more audio examples ]

I hope you know have a fist impression what Eqoder can do. In the next video, I will show you, how to use Eqoder as 
a sound design tool.


## 2 shows examples sound design (drum sounds, attack changer, long term modulation)h drum 

In this video I will show you one possible way to use Eqoder as a sound design tool. It acts like another filter module in your favourite synth. 

Lets start with some drum and percussion sounds. The source audio is pink noise from Surge XT. But this time we will use the envelope and adapt it to the desired sound.

we use the same midi keyboard as input. You can see the routing here.

Test some sounds


## 3 show examples music modulation (play a melody and play others on EQoder)

In this video I will show you another way to use Eqoder. For this application we need our audio workstation. For me  this is Cubase, but more or less all will work.

Lets start with a short, kind of slowly melody. The instrument nes to be rich in sound or a little noisy. Ok nice enoough. Lets record something.

And now we modulate this signal with another melody on top of its own sound with Eqoder.


## 4 how it works
In this video, I will show you the internals of EQoder. This is mostly digital signal processing basics with some small tricks.
Lets start with the basics.  Eqoder is a cascade of peak filter: one filter after the next. Each filter is a second order filter but not in the typical implementation of direct form 1 or 2. From the beginning  I knew each filter is time-variant. We change the frequency by playing the keyboard and each filter has its own envelope. Of course you could design new filter coefficients for each change, but this is not very efficient. 
Instead I used a so-called decoupled allpass structure, which was presented by Regalia and Mitra in their paper
"Tunable Digital Frequency Response Equalization Filters"

The main disadvantage of this structure is that it is not symmetric if you amplify of reduce the gain at a given frequency. 
But this is no problem for EQoder, since I will only amplify the signal. 
Now to the main advantage. This filter is really decoupled. What does that mean? Each parameter, gain, frequency and Q factor is one coefficient in the internal structure. You don't compute any other parameter. And the internal allpass structure has several other advantages for example a very easy test for stability.

So the internal structure looks like this. 






The technical background (only for the nerds)



