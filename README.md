# hlCursors
Little program to create highlighted Xcursors versions for any Xcursor theme.

I started this project because I wanted to have a highlighted cursor under linux for making videos, and all I found online was: https://unix.stackexchange.com/questions/241082/highlight-mouse-for-screencasts-without-disturbing-workflow/411483#411483 where the primary answer only provided a highlight for the basic pointer cursor... Which was not enough.

The program creates copies of all the cursor files in a given directory, adding a highlight to every image in each file (multi size and animated cursors are therefore properly supported)

The code probably builds under any POSIX system that uses Xcursors. In the releases I also include the result of running the progam against the DMZ-White theme. 
