##Acknowledgement##
This was forked from https://github.com/jooray/motion-detection

##Dependency##
`opencv` - in Debian, install as below

    sudo apt-get install libopencv-dev

## Compilation

    cd motion_src
    cmake .
    make

## Usage

    ./bin/motion <input_video> <output_dir>

`motion` sub directory will be created in `<output_dir>`.

    Usage: motion INPUT_FILE_NAME OUTPUT_DIRECTORY [x1 y1 x2 y2 [pixel_change motion]]
    Default values: x1, y1 = 0; x2, y2 = width, height of the frame, pixel_change=5 motion=20


