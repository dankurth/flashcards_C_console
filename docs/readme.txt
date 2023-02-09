The document FLASH.TXT, changes.txt and the source code (under src/) were written between 1991 and 1992 so are now more than 30 years old so while the description in FLASH.TXT still applies the addresses are no longer valid. Will update in any subsequent revisions.

The source code originally compiled using TurboC (I forget which version). 

The current build was on a DOSBox on Debian GNU/Linux 11 (bullseye) running on an old Dell Optiplex 780.

The compiler used for this build on DOSBox is TurboC 2.0 (installed to C:\TC within DOSBox) and TASM from Borland C++ 3.1 (the TurboC I installed did not include TASM so calls to that for my inline assembler code failed during build). Fortunately I already had installed a Borland C++ which did have TASM, so temporarily changed PATH in my .dosbox config so TCC would find it's own artifacts but if not found would then search within BorlandC, as follows:

...
[autoexec]
# Lines in this section will be run at startup.
# You can put your MOUNT lines here.
mount c ~/DOSBox
#path %PATH%;C:\BORLANDC\BIN
path %PATH%;C:\TC;C:\BORLANDC\BIN
c:

I'd also installed Windows 3.1 prior to the Borland C++ install because Borland C++ expected it to be there. Not sure that's really needed.
Before building it is necessary to change to the folder which contains the source code.
The *.INC files contain inline assembler code so must be compiled from command line.
The command line command to build each of the four *.INC files is "TCC *.INC" (e.g.: "TC INTSTR.INC").
Once the .INC files are built then FCM.C can be compiled using TC (the IDE) by opening FLASH.PRJ and selecting "Build All".
I've saved the IDE options as TCCONFIG.TC, which can be retrieved before building using the IDE to do so.
It MAY compile if contained in a virtual machine "shared" folder, but cannot be run from there.

Links:
https://archive.org/details/borland-turbo-c-v2.0	TurboC 2.0
https://archive.org/details/bcpp31			Borland C++ 3.1
https://winworldpc.com/product/borland-c/30		Windows 3.1
https://github.com/johangardhage/dos-bcdemos		DOSBox info including very useful method to easily install from img's on it



