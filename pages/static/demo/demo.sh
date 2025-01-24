#$ delay 30

 # Fetch PCRE2 with 'git clone', or use curl/wget to download a release.
#$ wait 100
 # Here, let's use git to check out a release tag:
#$ wait 1000

 git clone https://github.com/PCRE2Project/pcre2.git ./pcre2 \
    --branch pcre2-$PCRE2_VERSION \
    -c advice.detachedHead=false --depth 1
#$ expect ~home: \$ 
#$ wait 1000

 # Now let's build PCRE2:
#$ wait 500
#$ send \x20(cd ./pcre2; \\\n
#$ expect >
#$ send \x20\x20\x20\x20cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -B build; \\\n
#$ expect >
#$ send \x20\x20\x20\x20cmake --build build/)
#$ wait 1000
#$ send \n
#$ expect ~home: \$ 
#$ wait 1000

 # Great, PCRE2 is built.
#$ wait 500
 # Here's a quick little demo to show how we can make use of PCRE2.
 # For a fuller example, see './pcre2/src/pcre2demo.c'.
 # Try this pre-prepared sample code:
#$ wait 1000

#$ send \x20cat demo.c
#$ wait 1000
#$ send \n
#$ expect ~home: \$ 

 # Compile the demo:
#$ wait 100
 gcc -g -I./pcre2/build -L./pcre2/build demo.c -o demo -lpcre2-8
#$ expect ~home: \$ 
#$ wait 2000

 # Finally, run our demo:
#$ wait 100
 ./demo 'c.t' 'dogs and cats'
#$ expect ~home: \$ 
#$ wait 100

#$ send \x20# We fetched, built, and called PCRE2 successfully! :)
#$ wait 1500
#$ send \n

#$ sendcontrol d
