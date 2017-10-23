## Acknowledgement
This was forked from https://github.com/jooray/motion-detection

## Dependency
`opencv` - in Debian, install as below

    sudo apt-get install libopencv-dev

## Compilation

    cmake .
    make

## Usage

    ./motion <input_video> <output_dir> [<changes_threshold> <motion_deviation>]

Defaults:
- changes_threshold = 10
- motion_deviation = 20
