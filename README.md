# AA_lifegame
CUI based lifegame 
![CANNON](./AAlife.gif "lifegame")

## How To Build
just
```
 % make
```
## Usage
```
./lifegame [-h] [-w wait(in ms)] [-c colormap(1 - 32)] [filename]
```
By default, initial scene is will be randomly generated.
If a "filename" is specified, the initial scene will be the content of that file.

following options are available:

```
-c [colormap]  Enable color.
               If integer number is specified, colorsmap specified colormap. 
               The colormap is integer number from 1 to 32.
               if no colormap is specified, randomly choosen.

   -w [wait]   Set animation cycle to wait(in ms)

   -h          show help (colormap list is also displayed)