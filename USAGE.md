mkdir build && cd build && cmake ..
make -j8
./snoop {arguments}
./snoop -f ../input.txt -n 2