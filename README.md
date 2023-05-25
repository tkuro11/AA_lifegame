# AA_lifegame
CUI based lifegame 

## How To Build
just
 % make

## Usage
 ./lifegame [-w wait(in ms)] [-c colormap(1 - 15)] [filename]

By default, initial scene is will be randomly generated.
If a "filename" is specified, the initial scene will be the content of that file.

following options are available:

   **-c** [*colormap*]    Enable color.
                    If integer number is specified, colorsmap specified colormap. 
                    The colormap is integer number from 1 to 15.
                    if no colormap is specified, randomly choosen.
   **-w** [*wait*]        Set animation cycle to *wait*(in ms)

