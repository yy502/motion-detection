## Acknowledgement
This was forked from https://github.com/jooray/motion-detection

## Dependency
`opencv` - in Debian, install as below

    sudo apt-get install libopencv-dev

## Compilation

    cd motion_src
    cmake .
    make

## Usage

    ./bin/motion <input_video> <output_dir> [<changes_threshold> <motion_deviation>]

Defaults:
- changes_threshold = 5
- motion_deviation = 20


