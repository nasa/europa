@echo on

REM  @author Mark Roberts
REM  A simple batch file to ensure that all required libraries are in the distribution lib directory.
REM  Run this from the examples\xxx directory, where xxx is the example to be run

SET PATH=%ANT_HOME%\bin;..\..\lib

ant