# flash

![screenshot](docs/assets/images/runFlashOnDOSBox.png?raw=true)

This program was developed as a study aid. It works like flash cards but with the added advantage that it keeps track of which cards you have problems with and shows only those cards. Cards that are answered correctly the first time are marked and are not shown again until cleared by the user. Those that are answered incorrectly are shown again in random order until they are answered correctly, and they will keep showing up in new sessions until they are answered right the first time in one of those sessions.

Questions are quizzed using the "honor system" where user is simply asked if they got it right.
You can use the cards provided or you can make your own. When you miss a question you will be shown the correct answer. The computer will keep track of your progress and show you your score.

Any word editor may be used to make a data file provided that it is capable of saving the file as a text file without hidden or special characters.

An example of a data file is illustrated below:

```
1 one (in Spanish)=uno} 
1 you (plural, familiar, in Spanish):
  a. tú
  b. usted
  c. vosotros
  d. vosotras
  e. vos
  =c}
0 The original 13 colonies of the United States were=
New Hampshire, Massachusetts, Rhode Island, Connecticut, New York, New Jersey, Pennsylvania,
Delaware, Maryland, Virginia, North Carolina, South Carolina, and Georgia.}
```
  
The single digit preceding the question is a marker. This number records if users have answered the question correctly the first time they encountered it in any given session (1 if true, 0 if false).  Following the first space is your first question, delimited by the "=" sign. Following the equal sign is the answer. Following the answer is the "}" character, which signals the end of that answer. Questions and answers may span more than one line, and are each limited to 480 characters.

If you make a new data file or add cards to an existing file you must use the same format for them as in the example given above, eg "0 loco=crazy} followed by a new line".

Due to their usage here as delimiters the characters "=" and "}" cannot currently be used as part of the content of either questions or answers.

A study file can be opened when starting flash by adding it as an argument on the command line, as in "flash demo.rec" (without the quotation marks).

You can specify the number of questions to work on per session by using the "-m" option, as in "flash -m 10" or "flash -m 1512".  The default is 100. If you have an extremely large study file and lengthy questions and answers and are using using a version of this program compiled for DOS it is possible you could get a message informing you that you've run out of memory, in which case simply run it again specifying a lower maximum number of records per session.

The program may reside in a separate directory, but MUST be started up in the same directory as your study files (you must change to the directory where your study files are before running flash, then you can start it up as in "flash" or, if it is in another directory eg /home/p/kurth then "/home/p/kurth/flash".  You can specify which study file to use when starting it up, eg "/home/p/kurth/flash/mystudyfile.rec", but you must be in the same directory as the study files (I'll add directory switching capablility later).

The score shown reflects how many questions you said were answered correctly (the FIRST TIME you saw them) during a session. Questions answered wrong are not counted. Questions answered correctly after previous attempts also are not counted. The final score reflects how many you would have gotten if you had been given only one attempt at each question, just as in a test on paper. There is also a percentage display, which is the score you would have received expressed as a percentile of total correct first time in session divided by total number of questions in the session.

