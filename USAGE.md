## mkdir build && cd build && cmake ..

## make -j8

## ./snoop {arguments}



## -f inputfilename

## -n nthreads (default 1)

## -i numberofCachelines per processors (default 30)

## ./snoop -f ../input.txt -n 2 -i 30

## make &&  ./snoop -f ../input.txt -n 2 -i 16 > output.txt

## make && ./snoop -f pinatrace.out -n 129 -i 16 > cs_64.txt