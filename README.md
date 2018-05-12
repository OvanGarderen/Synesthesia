Synesthesia
===========

The goal of this project is to generate animated so-called _replications_ from still images.
To do this we plan to blend various algorithms and image-manipulation techniques together,
such as distance based HSV-adjustments, Shepards-transforms, DeepDream.
The end-game would be to link the blending and parameter-choices to some kind of other
signal, such as a musical one, which would create a form of synesthesia.

![An example of a replication made with this program.](https://github.com/OvanGarderen/Synesthesia/blob/master/finished/output.gif?raw=true)

Usage
-----

Make the main executable:
```
make undulate
```
then run with
```
./undulate images/enchanted_forest.jpg
```
You'll be able to select the amount of edge detection used by hand, values of 100, 200 are
usually good.

When running on different images its advisable to remove the some cache files in the `temp/` directory, otherwise
the breathing motion of another image is used.
