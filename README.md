# theremin
Kevin? Webcam Piano/Theremin

This was originally a project for my CS1L class at UCSB were everyone got to make up a project and (with instructor approval) get graded for it.

Originally, it was meant to replicate the functionality of a theremin, but limitations in my knowledge of computer vision at the time and compute time cause me to switch to a piano format.

The executable is names wind_waker since the only song it can realistically play is Zelda's lullaby (I didn't add the full set of notes since it was easier to learn to play it that way).

On the functionality:

There are two threads. The first uses ALSA to play notes generated with additive synthesis. Envelopes were never implemented as it was agreed upon by members of the class that cutting the sound wave off sounded crisper. Envoloping will be added if I ever put in a separate volume control.

The second thread uses openCV to get the webcam image and check the piano-key regions for the presence of the finger of the pianist.

How to use: Place 
