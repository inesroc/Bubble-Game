# Project

### Inês Rocha (up201606266)
#### Computer Vision, Faculdade de Ciências (2019)

My project consists of a game. Several circles appear in the screen that needs to disappear and with the help of a ball on top of a stick that the player has in his hand, we can pass through them and make them disappear.
It was used the program objectTrackingTutorial.cpp developed by Kyle Hounslow in 2013 for the base of the program, and for better tracking of the ball, it was used as an adaptation of the tracking algorithmic found in the article Simple Object Tracking With OpenCV by Adrian Rosebrock.

# Calibration

To calibrate the game appears a blue circle in the middle of the screen and the player has to
position the ball inside that circle.

Figure 1 - Calibration

![image](https://user-images.githubusercontent.com/44119905/144416089-1e95a37e-09eb-49e2-9407-8c43cb3f3b2f.png)


During the calibration time, the program performs the following steps:

1. Convert the frame from BGR (OpenCV represents RGB as BGR) to HSV

It was needed the most information about the color of the ball that we could get so that we could remove the background and the only thing left would be the ball. Because RGB doesn’t have a good color description, HSV was used.

2. Draw the blue circle and the 4 points were the HSV values will be extracted 

To be able to get only the ball we need the minimum and maximum valor of HSV. This minimum and maximum creates an interval of values that gets all the possible
combinations of HSV that still belong to the ball. On the ball, we can have shadows and highlights and we will still be able to recognize it.
The minimum value of HSV is the lowest HSV values from the four points, and the maximum is the following HSV value:
H:55, S:255, V: 255

The saturation and value are the maximum that the HSV in OpenCV allows us, and the value of hue was found by testing several values and seeing which one was the best.

# Prepare Frame

To be able to detect the ball we need to do several steps to prepare the frame for tracking.

1. Invert the frame - This step is not needed to perform the tracking. It’s only to make the player play in a natural and normal way. The frame is inverted so that it behaves like a mirror.

2. Convert from BGR to HSV - Same reason and the one from the point above.

3. Apply the HSV values from the calibration to the frame - This is done using the function inRange that creates a binary image. If the value is between he minimum and the maximum HSV values it will be marked as one, else it will be marked as zero.

4. Dilate the image - Because we are using a tennis ball that has stripes of a different color, they will appear black in the binary image creating a hole.

Figure 2 – Example of the ball

![image](https://user-images.githubusercontent.com/44119905/144416392-7f40352c-f489-4916-8f18-2c0ba0b9a7af.png)

To prevent this, the image will be dilated so that all those black pixels in the middle of the ball will be transformed into white pixels. The image is dilated using the function morphologyEx. This function needs a kernel that allows the function to know if it needs to change a pixel or not. A pixel is marked as one if at least one pixel under the kernel is one.

Figure 3 – Dilation with a kernel 10x10 vs a kernel with 40x

![image](https://user-images.githubusercontent.com/44119905/144416505-bd14ad93-9c71-4981-bb0b-6a5743f652f9.png)

It was tested using a kernel of size 10x10 but is not large enough to cover the lines so it was changed to a size of 40x40.

# Track Objects

To track the ball was used the function findContours. This function was used because it can give us the area and the center points of the ball and that is needed for the game to work. findContours(InputOutputArray image, OutputArrayOfArrays contours, OutputArray hierarchy, int mode, int method)
* Image – the binary image
* Outputs – a vector of points
* Hierarchy – contains information about the topology of the image
* Mode – retrieves all of the contours without establishing any hierarchical relationships.
* Method - This is the contour approximation method. It was tested two options:
  CV_CHAIN_APPROX_NONE – stores all the contour points
  CV_CHAIN_APPROX_SIMPLE – stores the minimal possible amount of points that represent the contour. For example, if we have a rectangle, we get the following results

Figure 4 - Rectangle with CV_CHAIN_APPROX_NONE vs Rectangle with CV_CHAIN_APPROX_SIMPLE

![image](https://user-images.githubusercontent.com/44119905/144416695-71f2ad51-42ba-4c51-afd6-b9acf73cf2d6.png)

And this is the result with our ball:

Figure 5 – Ball with CV_CHAIN_APPROX_NONE vs Ball with CV_CHAIN_APPROX_SIMPLE

![image](https://user-images.githubusercontent.com/44119905/144416836-ddfcc013-f43c-4159-bdb1-1e017d85752c.png)

Because we are working with round contours there is no significate different between the two methods so we will use the CV_CHAIN_APPROX_SIMPLE.

After getting the contours we need to iterate through them and transform them into moments. Moments is a definition borrowed from physics that allows as to know the area and the center point of the object defined by the contours.
With the value of the area, we can eliminate immediately objects that are too small to be the ball that we are tracking in our program, any moment small than 67x67 is discarded. After having a list of center points of the possible moments could our ball, we pass them to another function that assigns an id to them. 

# Centroid Tracker

To be able to have two balls and following them we need to rely on the Euclidean distance. For each frame, we will calculate the distance between all the old objects and the new objects. We choose the smallest distance and that new object will receive the respective id. Because the game doesn’t allow to have more than 2 balls. If more than 2 objects appear on the screen, the third will be ignored.

# Game Mechanics

## Bubbles
* Are save in a map
* Every time a new level starts the map is clean and populated with the new bubbles
* On each frame, we check if the player has touched on a bubble and if it does it’s added to the score 5*current level points and remove the bubble

## Trail

Every time a player catches a bubble a circle is added to their trail. The trail is a queue of points that when it’s printed is printed as an overlay.

To print the trail transparent, we need to define an alpha value, and then sum the circle with the image Bubbles*alpha + image*(1-alpha) = new image


