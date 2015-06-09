#OpenCV C++ Motion Detection

This is based on work by [Cedric Verstraeten](https://blog.cedric.ws/opencv-simple-motion-detection), also see his [Kerberos.io project](https://www.kerberos.io) for a very nice web interface and camera monitoring interface for Raspberry Pi.

This project handles a different problem: you have a bunch of videos and you only want the parts with motion.

#Installation 

Install OpenCV, for example on Mac using Homebrew:

    brew install homebrew/science/opencv --with-ffmpeg

Then compile:

    cd motion_src
    cmake .
    make

Now you can try it out:

    ./bin/motion input.avi ~/tmp/motion/output 431 303 541 513

This will detect motion in a box starting at (431, 303) and lower right corner of (541, 513). You can also adjust additional parameters:


    Usage: motion INPUT_FILE_NAME OUTPUT_DIRECTORY [x1 y1 x2 y2 [pixel_change motion]]
    Default values: x1, y1 = 0; x2, y2 = width, height of the frame, pixel_change=5 motion=20


