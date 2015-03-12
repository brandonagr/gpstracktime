## TrackTime ##
TrackTime is a budget data logging solution for people running road courses. Assuming someone already has a laptop that they can securely mount in their car while doing laps they only need to get a consumer grade GPS receiver.

### Data Logger ###
The data logger runs continuously while you are the track and automatically begins recording your session when you get up to speed. It will stop recording when you are done, so there is no hitting a button everytime you go out or wondering if it's working.

### Lap Analysis ###
The lap analysis breaks up the GPS record into individual laps and shows you the line around the track. You can also replay various laps and compare your speed at different points along the track.

In order to account for GPS drift I currently have several ideas. First off, the only assumption that I make is that drift is negligible in the 2-3 minute time frame that it would take to make a single lap of the track. This means that all readings taken from a single lap are accurate relative to each other.

In order to create an accurate map of the track I set the GPS receiver on a fixed landmark right before and after creating the map. This allows me to sort of emulate a differential GPS setup(hopefully) and eliminate some of the drift that occurred during the map making(assuming that drift happens linearly over time). If I know the amount of drift then a simple translation will correct each data point depending on when it was taken.

In order to align laps captured on different days where the drift could be upwards of 50 meters(making direct comparisons impossible) I will take an iterative approach. Assuming they are all good laps where the car did not go off track, then all data points should be inside the track path created earlier. For every data point outside of the track, I will average together the distance needed to move the whole lap's data back onto the track. Given the length and complexity of the track itself, I think that this will likely produce good results.

Here is an example of the magnitude of the drift error in just one day over 3 sessions, all of the line in the pic should be right on top of each other
![http://i283.photobucket.com/albums/kk312/brandonagr/TTVis_drift.jpg](http://i283.photobucket.com/albums/kk312/brandonagr/TTVis_drift.jpg)

After some more research, it turns out that the GPS drift/alignment problem might be a bit more difficult, here is an example of a recorded lap that aligns at part of the track and not another. Meaning there is some relative error that is coming from either drift or just inaccuracy in the GPS unit in that it can't keep up with the car while going around at ~100 MPH
![http://i283.photobucket.com/albums/kk312/brandonagr/alignment_back_straight.jpg](http://i283.photobucket.com/albums/kk312/brandonagr/alignment_back_straight.jpg)
![http://i283.photobucket.com/albums/kk312/brandonagr/alignment_2_3.jpg](http://i283.photobucket.com/albums/kk312/brandonagr/alignment_2_3.jpg)
Getting that data to line up correctly is currently the main problem I am facing. as a 20 meter error in one section of the track makes replaying the data not very useful.

![http://i283.photobucket.com/albums/kk312/brandonagr/track_textured.jpg](http://i283.photobucket.com/albums/kk312/brandonagr/track_textured.jpg)
![http://i283.photobucket.com/albums/kk312/brandonagr/auto_aligned.jpg](http://i283.photobucket.com/albums/kk312/brandonagr/auto_aligned.jpg)

### GPS Drift ###

I also decided to get more information on effects of GPS Drift. I used the [GPS Toolkit](http://www.gpstk.org/bin/view/Documentation/WebHome)'s rinexpvt to convert the freely available [CORS](http://www.ngs.noaa.gov/CORS/Data.html) data(in RINEX format) into positions to get the amount of GPS drift during a certain time period. This allows for some post processing differential correction of the GPS data I collected.

This is a graph of the 30 minute window that I was mapping the track during. My main worry is the amount of drift change within the 10 minute window that I was driving around the track. From this result the error should be less than half a meter over that amount of time, plenty small to produce good results.
![http://i283.photobucket.com/albums/kk312/brandonagr/gps_error.jpg](http://i283.photobucket.com/albums/kk312/brandonagr/gps_error.jpg)

### Credits ###
Thanks go to Aaron Averett for much advice regarding GPS and surveying.