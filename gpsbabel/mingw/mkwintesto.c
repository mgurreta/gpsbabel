/*
    File for "converting" GPSBabel's testo shell script into
	MS Windows NT/2000/XP command script.

	It is limited to:
	- testo using the shell variable PNAME for the executable program being tested
	- testo using ${TMPDIR} for the temporary directory in which test files are created
	- testo using compare as the name of a shell function for comparing test results
	- no other shell script conversion is performed apart from whole line comments;
	  unconverted script is discarded, not even included as comments in the output

    Copyright (C) 2003 Mark Bradley, mrcb.sf.gpsb@osps.net

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA
 */


#include <stdio.h>
#include <string.h>

#define	LINELENGTH	200
#define	MYNAME		"MkWinTesto"

/* ------------------------------------------------------------------------------------ */
int f_outputLine (
	char	*pcWhat,
	FILE	*pfWhere)
{
	int		iLength;
	int		iThisChar;

	/* ===========================
	   Return 0 = output has new line
	   Return 1 = line ended on \ for continuation and no new line
	*/

	iLength = strlen(pcWhat);
	if (iLength > 2) {
		if ((*(pcWhat+iLength-3) == '\\') &&
			(*(pcWhat+iLength-2) == '\r') &&
			(*(pcWhat+iLength-1) == '\n')) {

			for (iThisChar=0; iThisChar < iLength-3; iThisChar++)
				fputc(*(pcWhat+iThisChar), pfWhere);
			return 1;
		}
	}
	if (iLength > 1) {
		if ((*(pcWhat+iLength-2) == '\\') &&
			(*(pcWhat+iLength-1) == '\n')) {

			for (iThisChar=0; iThisChar < iLength-2; iThisChar++)
				fputc(*(pcWhat+iThisChar), pfWhere);
			return 1;
		}
	}
	if (iLength > 0) {
		if (*(pcWhat+iLength-1) == '\\') {

			for (iThisChar=0; iThisChar < iLength-1; iThisChar++)
				fputc(*(pcWhat+iThisChar), pfWhere);
			return 1;
		}
	}

	fputs(pcWhat, pfWhere);
	fputs("\r\n", pfWhere);
	return 0;
}

/* ------------------------------------------------------------------------------------ */
int main(
	int argc, 
	char *argv[])
{
	char	acLineIn[LINELENGTH];
	char	acLineOut[LINELENGTH];

	int		iThisChar;
	int		iStart;
	int		iTarget;
	int		iTranslateQuotes;
	int		iQuoteCount;
	int		iPrevLineContinues = 0;
	int		iEchoLevel = 0;

	FILE	*pfTestoIn;
	FILE	*pfTestoOut;

	/* =========================== */

	if (argc < 2) {
		fatal(MYNAME ": needs a single parameter, the (testo) file to convert\n");
	}
	pfTestoIn = fopen(argv[1], "rb");

	if (pfTestoIn == NULL) {
		fatal(MYNAME ": %s for reading\n",argv[1]);
	}
	else {
		pfTestoOut = fopen ("wintesto.cmd", "wb");
		if (pfTestoOut == NULL) {
			fatal (MYNAME ": wintesto.cmd for writing\n");
		}
		else {

			/* Output the .CMD preamble */
			f_outputLine("@echo off", pfTestoOut);
			f_outputLine("REM", pfTestoOut);
			f_outputLine("REM Simple Windows NT/2000/XP .cmd version of GPSBabel testo script", pfTestoOut);
			f_outputLine("REM", pfTestoOut);
			f_outputLine("", pfTestoOut);
			f_outputLine("SET TMPDIR=%TEMP%\\WINTESTO", pfTestoOut);
			f_outputLine("MKDIR %TMPDIR% 2>NUL:", pfTestoOut);
			f_outputLine("", pfTestoOut);
			f_outputLine("GOTO :REALSTART", pfTestoOut);
			f_outputLine("", pfTestoOut);
			f_outputLine(":COMPARE", pfTestoOut);
			f_outputLine("SET PARAM1=%1", pfTestoOut);
			f_outputLine("SET PARAM2=%2", pfTestoOut);
			f_outputLine("REM Test if param2 was a dir rather than a file, if so add a \\* to make fc work", pfTestoOut);
			f_outputLine("FOR %%A IN (%2) DO IF \"d--------\"==\"%%~aA\" SET PARAM2=%2\\*", pfTestoOut);
			f_outputLine("FOR /f \"delims=\" %%a IN ('fc %PARAM1% %PARAM2%') DO IF \"x%%a\"==\"xFC: no differences encountered\" GOTO :EOF", pfTestoOut);
			f_outputLine("REM Show the first 5 lines of difference", pfTestoOut);
			f_outputLine("fc /LB5 %PARAM1% %PARAM2%", pfTestoOut);
			f_outputLine("ECHO %* are not the same (first 5 differences above) - pausing. ^C to quit if required", pfTestoOut);
			f_outputLine("PAUSE", pfTestoOut);
			f_outputLine("GOTO :EOF", pfTestoOut);
			f_outputLine("", pfTestoOut);
			f_outputLine("REM ==================================", pfTestoOut);
			f_outputLine("", pfTestoOut);
			f_outputLine(":SORTandCOMPARE", pfTestoOut);
			f_outputLine("SORT <%1 >%TMPDIR%\\s1", pfTestoOut);
			f_outputLine("SORT <%2 >%TMPDIR%\\s2", pfTestoOut);
			f_outputLine("CALL :COMPARE %TMPDIR%\\s1 %TMPDIR%\\s2", pfTestoOut);
			f_outputLine("GOTO :EOF", pfTestoOut);
			f_outputLine("", pfTestoOut);
			f_outputLine("REM ==================================", pfTestoOut);
			f_outputLine("", pfTestoOut);
			f_outputLine(":REALSTART", pfTestoOut);
			f_outputLine("", pfTestoOut);



			while (! feof(pfTestoIn)) {
				/* Read in the next line or stop if done */
				fgets(acLineIn, LINELENGTH-1, pfTestoIn);
				if (acLineIn == NULL) break;
				
				/* Is the whole line a comment? Replace the hash with REM and output the rest */
				if (acLineIn[0] == '#') {
					acLineOut[0]='\0';
					strcat (acLineOut,"REM");
					/* Strip out any ending new lines */
					for (iThisChar=1; iThisChar<LINELENGTH; iThisChar++) {
						if ((acLineIn[iThisChar] == '\0') ||
							(acLineIn[iThisChar] == '\n') ||
							((acLineIn[iThisChar] == '\r') && (acLineIn[iThisChar+1] == '\n'))) {

							acLineOut[iThisChar+2] = '\0';
							break;
						}
						acLineOut[iThisChar+2] = acLineIn[iThisChar];
					}
					if (iEchoLevel > 0) {
						f_outputLine("@echo off", pfTestoOut);
						f_outputLine("@echo.", pfTestoOut);
						iEchoLevel = 0;
					}
					iPrevLineContinues = f_outputLine(acLineOut, pfTestoOut);
				}
				/* Are we near the top of testo where the program variable is defined? */
				else if (strncmp("PNAME=${PNAME:-",acLineIn,15) == 0) {
					acLineOut[0]='\0';
					strcat (acLineOut,"SET PNAME=");
					/* Copy the rest of the PNAME assignment stopping at a close } or EOL */
					for (iThisChar=15; iThisChar<LINELENGTH; iThisChar++) {
						if ((acLineIn[iThisChar] == '}') ||
						    (acLineIn[iThisChar] == '\0') ||
							(acLineIn[iThisChar] == '\n') ||
							((acLineIn[iThisChar] == '\r') && (acLineIn[iThisChar+1] == '\n'))) {
							acLineOut[iThisChar-5] = '\0';
							break;
						}
						if (acLineIn[iThisChar] == '/') {
							acLineOut[iThisChar-5] = '\\';
						} else {
							acLineOut[iThisChar-5] = acLineIn[iThisChar];
						}
					}
					if (iEchoLevel > 0) {
						f_outputLine("@echo off", pfTestoOut);
						f_outputLine("@echo.", pfTestoOut);
						iEchoLevel = 0;
					}
					iPrevLineContinues = f_outputLine(acLineOut, pfTestoOut);
					if (iPrevLineContinues == 1) f_outputLine("", pfTestoOut);
					iPrevLineContinues = f_outputLine("IF NOT EXIST %PNAME% ECHO Can't find %PNAME%&& GOTO :EOF", pfTestoOut);
					/* fputs("\r\n", pfTestoOut); */
				}
				else {
					/* Every other line.... */
					iStart = 0;
					iTarget = 0;
					iTranslateQuotes = 0;
					iQuoteCount = 0;
					acLineOut[0] = '\0';

					/* Is this one of the test sequences mostly (all?) starting with a cleanup? */
					if (strncmp("rm -f ",acLineIn,6) == 0) {
						if (iEchoLevel > 0) {
							f_outputLine("@echo off", pfTestoOut);
							f_outputLine("@echo.", pfTestoOut);
							iEchoLevel = 0;
						}
						iStart = 6;
						strcat(acLineOut, "DEL ");
						iTarget = 4;
					}
					/* Is this one of the test sequences where the program is run? */
					if (strncmp("${PNAME} ",acLineIn,9) == 0) {
						iStart = 9;
						iEchoLevel++;
						strcat(acLineOut, "%PNAME% ");
						iTarget = 8;
					}
					/* Is this one of the test sequences where we compare the rest? */
					if (strncmp("compare ",acLineIn,8) == 0) {
						if (iEchoLevel > 0) {
							f_outputLine("@echo off", pfTestoOut);
							f_outputLine("@echo.", pfTestoOut);
							iEchoLevel = 0;
						}
						iStart = 8;
						strcat(acLineOut, "CALL :COMPARE ");
						iTarget = 14;
					}
					/* Is this one of the test sequences where we compare the rest? */
					if (strncmp("sort_and_compare ",acLineIn,17) == 0) {
						if (iEchoLevel > 0) {
							f_outputLine("@echo off", pfTestoOut);
							f_outputLine("@echo.", pfTestoOut);
							iEchoLevel = 0;
						}
						iStart = 17;
						strcat(acLineOut, "CALL :SORTandCOMPARE ");
						iTarget = 21;
					}
					/* Is this one of the test sequences where we prepare some data? */
					if (strncmp("echo \"",acLineIn,6) == 0) {
						if (iEchoLevel > 0) {
							f_outputLine("@echo off", pfTestoOut);
							f_outputLine("@echo.", pfTestoOut);
							iEchoLevel = 0;
						}
						iStart = 6;
						strcat(acLineOut, "ECHO ");
						iTarget = 5;
						iTranslateQuotes = 1;
						iQuoteCount = 1;
					}
					if ((iStart > 0) ||
					    (iPrevLineContinues == 1)) {

						if (iStart == 0) {
							/* Didn't match, so can only possibly be a continued line
							   Skip spaces, then process the rest of line as "normal" */
							for (iThisChar=0; iThisChar<LINELENGTH; iThisChar++) {
								/* end of line checks */
								if ((acLineIn[iThisChar] == '\0') ||
									(acLineIn[iThisChar] == '\n') ||
									((acLineIn[iThisChar] == '\r') && (acLineIn[iThisChar+1] == '\n'))) {
									iStart = iThisChar;
									break;
								}
								/* Stop skipping spaces and tabs if this isn't one (arg!) */
								if ((acLineIn[iThisChar] != ' ') &&
									(acLineIn[iThisChar] != '\t')) {
									iStart = iThisChar;
									break;
								}

							}	/* for ... skipping spaces on a continued line */
						}	/* if... is this a continued line? */


						/* Need to replace the shell variable replacement syntax with Windoze one
						   and swap slashes in directory separators
						*/

						for (iThisChar=iStart; iThisChar<LINELENGTH; iThisChar++) {
							if ((acLineIn[iThisChar] == '\0') ||
							    (acLineIn[iThisChar] == '\n') ||
								((acLineIn[iThisChar] == '\r') && (acLineIn[iThisChar+1] == '\n'))) {
								acLineOut[iTarget+iThisChar-iStart] = '\0';
								break;
							}
							if (acLineIn[iThisChar] == '"') {
								/* Are we starting quotes or in quotes?
								   Either way, don't copy out the quotes themselves */
								iQuoteCount =  (iQuoteCount > 0) ? 0 : 1;
								iTarget--;
								continue;
							}
							if (acLineIn[iThisChar] == '%') {
								if (iQuoteCount == 1) {
									/* Need to double up the number of %s */
									acLineOut[iTarget+iThisChar-iStart] = '%';
									iTarget++;
								}
								/* This also caters for where we're not in quotes, 
								   so must just copy the % once */
								acLineOut[iTarget+iThisChar-iStart] = '%';
								continue;
							}
							if (acLineIn[iThisChar] == '>') {
								if (acLineIn[iThisChar-1] == ' ') {
									/* Need to remove any spaces between echo and redirection
									   as NT/2000/XP adds this to the output and mostly this is NOT wanted */
									iTarget--;
								}
								acLineOut[iTarget+iThisChar-iStart] = '>';
								continue;
							}
							if (strncmp("${TMPDIR}",acLineIn+iThisChar,9) == 0) {
								strcpy(acLineOut+iTarget+iThisChar-iStart,"%TMPDIR%");
								/* %TMPDIR% is one char shorter than ${TMPDIR} */
								iTarget--;
								/* skip forward to the end of the string matched
								   (less one as the loop will add one) */
								iThisChar += 8;
							} else if (acLineIn[iThisChar] == '/') {
								acLineOut[iTarget+iThisChar-iStart] = '\\';
							} else {
								/* part of a literal, so copy the text */
								acLineOut[iTarget+iThisChar-iStart] = acLineIn[iThisChar];
							}
						} /* for */
						if ((iEchoLevel == 1) && (iPrevLineContinues != 1)) {
							f_outputLine("@echo on", pfTestoOut);
							f_outputLine("@echo Testing...", pfTestoOut);
						}
						iPrevLineContinues = f_outputLine(acLineOut, pfTestoOut);
						/* fputs("\r\n", pfTestoOut); */
					}
					else {
						/* We didn't match the start of the line, so
						  - if blank, print it
						*/
						if (iEchoLevel > 0) {
							f_outputLine("@echo off", pfTestoOut);
							f_outputLine("@echo.", pfTestoOut);
							iEchoLevel = 0;
						}
						if ((acLineIn[0] == '\n') ||
							(acLineIn[0] == '\0') ||
							((acLineIn[0] == '\r') && (acLineIn[1] == '\n') && (acLineIn[2] == '\0'))) {
								iPrevLineContinues = f_outputLine("", pfTestoOut);
						}

					}	/* else...  didn't match a start of line - so check rest of line */

				}	/* else ... catchall to mathing things on the start of the line */

			}	/* while <stuff to read> */

			/* We're done */
			fclose(pfTestoIn);
			fclose(pfTestoOut);
		}
	}	
}
