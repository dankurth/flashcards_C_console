# flash

![screenshot](docs/assets/images/runFlashOnDOSBox.png?raw=true)

This program was developed as a study aid. It works like flash cards but with the added advantage that it keeps track of which cards you have problems with and shows only those cards. Cards that are answered correctly the first time are marked snd are not shown again until cleared by the user. Those that are answered incorrectly are reshown at random until they are answered correctly, and they will reappear in new sessions until they are answered right the first time in any of those sessions.

You can use the cards provided or you can make up your own to study anything that can be put on flash cards. When you miss a question you will be shown the correct answer. The computer will keep track of your progress and show you your score. In studying the cards you can use multiple choice or the honor system.

You can select a data file to use by specifying it on the command line, for example "flash span-eng.rec" to use the data file "span-eng.rec". In the honor system you guess the answer & then after you are shown the right answer YOU tell the computer wether you guessed it correctly or not. In "Type in Answer" you type in the answer (e.g. a letter corresponding to the correct one as in:

```
what is the third planet from the sun?
a. Venus
b. Earth
c. Both
d. The third planet was destroyed by Halley's comet 
e. None of the above
```

Any wordprocessor may be used to make a data file provided that it is capable of saving the file as an Ascii text file without hidden or special characters.  If in doubt as to wether a file fits this description you can perform a TYPE command on it and if it does not beep or print strange characters which are not part of the alphabet then it is probably an Ascii text file you can use.

An example of a data file with two questions is illustrated below:

```
1 one (in Spanish)=uno} 
1 you (plural, familiar, in Spanish):
  a. tú
  b. usted
  c. vostros
  d. vosotras
  e. vos
  =c}
0 The original 13 colonies of the United States were=
New Hampshire, Massachusetts, Rhode Island, Connecticut, 
New York, New Jersey, Pennsylvania, Delaware, Maryland, 
Virginia, North Carolina, South Carolina, and Georgia.}
```
  
The questions have only one right answer, "uno" for the first, "c" for the second, and "New Hampshire, Massachusetts, Rhode Island, Connecticut, New York, New Jersey, Pennsylvania, Delaware, Maryland, Virginia, North Carolina, South Carolina, and Georgia.". 

Questions can be quizzed using either the "honor system" where user is simply asked if they got it right, or by user typing in answer. Obviously the first two questions work for either mode but the last is better suited to honor mode (the "type in answer" mode is dumb. It expects an EXACT match, including order and punctuation and any unusual characters such as accent marks or umlauts).

The digit preceding the question is a marker. This number records if users answered the question correctly the first time they encountered it in any given session (1 if true, 0 if false).  Following the first space is your first question, delimited by the "=" sign. Following the equal sign is the answer. Following the answer is the "}" character, which signals the end of that answer. Questions and answers may span more than one line, and are each limited to 480 characters.

Due to their usage here as delimiters the characters "=" and "}" cannot currently be used as part of the content of either questions or answers.

You can specify the number of questions to work on per session by using the "-m" option, as in "flash -m 10" or "flash -m 1512".  The default is 100. If you have an extremely large study file and lengthy questions and answers and are using this program on a DOS machine it's possible that you could get a message informing you that you've run out of memory, in which case simply run it again specifying a lower maximum number of records per session.

If you make a new data file or add cards to an existing file you need to put put "0 " in front of the question, as in the first question & answer in the example above, eg "0 loco=crazy}".

A study file can be opened when starting flash by adding it as an argument on the command line, as in "flash demo.rec" (without the quotation marks).

The program may reside in a separate directory, but MUST be started up in the same directory as your study files (you must change to the directory where your study files are before running flash, then you can start it up as in "flash" or, if it is in another directory eg /home/p/kurth then "/home/p/kurth/flash".  You can specify which study file to use when starting it up, eg "/home/p/kurth/flash/mystudyfile.rec", but you must be in the same directory as the study files (I'll add directory switching capablility later).

The score shown reflects how many questions were answered correctly (the FIRST TIME) during this session. Questions answered wrong are not counted. Questions answered correctly after previous attempts also are not counted. The final score reflects how many you would have gotten if you had been given only one attempt at each question, just as in a test on paper. There is also a percentage display, which is the score you would have received expressed as a percentile of total correct first time in session divided by total number of questions in the session.

This version was built using MinGW (for Windows) and gcc (for Linux). 

