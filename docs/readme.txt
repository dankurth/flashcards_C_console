
The document FLASH.TXT is at least 27 years old (I wrote it in or before 1996) so while the description still applies the contact info is no longer valid. Will update it in any subsequent revisions.

The original version from 1995 or earlier was written to run on DOS only, compiled using TurboC.

The 1996 version ran on either DOS or Linux or Unix, compiled for DOS using Borland C++ 3.1.

This first 2023 version changes the 1996 version only insofar as it strips out the DOS specific code as builds for that code would not run on 64-bit Windows 10. Fortunately the streamlined code can now be cross-compiled as-is for Windows using MinGW.

The program has been built and tested recently (Jan 2022) on DOSBox and a 32-bit XP on VirtualBox (both hosted on Linux), and then tested but not built on Windows 10.

There are screenshots in the docs folder. Screenshots include:
Build on Linux (Debian 11)
Build on Windows (virtual XP SP2 on VirtualBox 6.1.26 on Linux
Run sequence on Linux (sequence according to time stamp in file names)

Currently the program has options for multiple choice and for answering by typing in. These options are confusing on the display and require showing a screen to select options prior to study. At this point I think they're more annoying than helpful so will probably be removed in the next tagged version.

Currently the program also has a few build warnings which will be cleaned up next.
