## mkdir build && cd build && cmake ..

## make -j8

## ./snoop {arguments}



## -f inputfilename

## -n nthreads (default 1)

## -i numberofCachelines per processors (default 30)

## ./snoop -f ../input.txt -n 2 -i 30

## make &&  ./snoop -f ../input.txt -n 2 -i 16 > output.txt

## make && ./snoop -f ../samples/thread64.out -n 65 -i 16 > ../sampleoutput/thread64.txt

# New version
## rm -rf build && mkdir build && cd build 
## cmake ..

## make && ./snoop -f thread64.out -n 65 -i 16 