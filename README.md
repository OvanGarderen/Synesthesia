Synesthesia
===========

The goal of this project is to generate animated so-called _replications_ from still images.
To do this we plan to blend various algorithms and image-manipulation techniques together,
such as distance based HSV-adjustments, Shepards-transforms, DeepDream.
The end-game would be to link the blending and parameter-choices to some kind of other
signal, such as a musical one, which would create a form of synesthesia.

Usage
-----

Make the main executable:
```
make undulate
```
then run with
```
./undulate image_sources/enchanted_forest.jpg
```
the executable creates some cache files in the `temp/` directory, so you may wish to
`rm temp/*` when running different images.

Optionally, you may specify a parameter between 0 and 256 that governs the edge detection,
lower values will create a more uniform effect, which may be uninteresting.
Higher values will find less edges, which gives clearer effects but may lead to a lot of open space.
For a lot of images values `50-100` should give good results:
```
./undulate image_sources/enchanted_forest.jpg 100
```
