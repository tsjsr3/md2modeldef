 ready for download

precompiled exe for commandline (and/or send to folder)

https://bitbucket.org/CBMFreak﻿/md2model ... deldef.exe

header file

https://bitbucket.org/CBMFreak/md2model ... r/anorms.h

main file

https://bitbucket.org/CBMFreak/md2model ... ter/main.c
 

what is it?

 

it is a tool designed to save you time when you are modding GZDoom and possibly other ZDoom derivatives that support MD2 models

 

so the story is... I use a lot of MD2 models in GZDoom for the time being and I got tired of writing the model definition files (modeldefs) by hand... so I made this quick tool that will read an md2 file and create a text file based on that file, that text file is a modeldef file and will contain a definition of all animations for that md2 file and create spitenames for each frame in each animation.. some of them will be commented out .. so it is suggested that the file be looked over after import to slade to ensure the animations you want are active... the tool aims to try and make sure the modeldef can be used by slade and GZDoom right away

the current public version that can be found at the link including full source ....
 

texture is read from MD2 file

path is read from MD2 file (path to texture, usually models\players)

spritenames are generated from animation names (yes yes... non animated MD2 models will not work, but this is not a tool for models without animation)

max number of animation sets is 10 in GZDoom apparently... so it skips (still written but marked as comments) 

some animation sets if there are more than 10, usually there are 16

resulting file is named texture name + .modeldef

jumps are handled as a "seperate" animation set regarding spritenames

spritenames are ... first 3 lettes of the animation set name plus a number

(numbered in packs of 10, usually long animations such as stand will be up to 40 frames.. ie... walk0-3 A-J)

If the numbering increases by more than 2 then I interpret that as a jump, so a jump from walk109 to walk201 would be detected

if the input file is named tris.md2 then the tool assumes there is a weapon.md2 with a weapon.pcx texture as well

( always assumes that an md2 file named tris.md2 has a weapon.md2 file with identical animations and otherwise it just maps out the animations for the md2 file)

example output could be

wal0 A 0 "walk01"

for tris.md2 and

wal0 A 1 "walk01"

for weapon.md2

ie. spritename spriteindex modelindex framename

 

IF there is no animation in the MD2 file then it will normally be saved (by maverick) with a single Frame called Frame0 or Fra0 in the modeldef


fixes, since first commit:

- naming ﻿now correct (now modeldef.<name> instead of <name>.modeldef)

- max assuming 9 sprites per animation, more can be used if the modeldef file is edited in slade (others are marked as comments)

- renames sprites that would otherwise be duplicates

- always uses first skin if present

- uses directory name if no name found elsewhere when using a file named tris.md2




as always... the newest version will be placed on bitbucket when I feel it is ready for primetime

- CBM from Doomworld and ZDoom forums