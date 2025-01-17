/*
 *
 *  Copyright (C) 1994-2021, OFFIS e.V.
 *  All rights reserved.  See COPYRIGHT file for details.
 *
 *  This software and supporting documentation were developed by
 *
 *    OFFIS e.V.
 *    R&D Division Health
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *
 *  Module:  dcmdata
 *
 *  Author:  Andrew Hewett
 *
 *  Purpose: Test if a file uses DICOM Part 10 format.
 *
 */

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include "dcmtk/ofstd/ofstdinc.h"

#include "dcmtk/dcmdata/dcmetinf.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcuid.h"    /* for dcmtk version name */

#define OFFIS_CONSOLE_APPLICATION "dcmftest+"

static char rcsid[] = "$dcmtk: " OFFIS_CONSOLE_APPLICATION " v"
  OFFIS_DCMTK_VERSION " " OFFIS_DCMTK_RELEASEDATE " $";

// ********************************************

OFBool opt_check = OFTrue ;
OFBool opt_filter_dicom = OFTrue ;
OFBool opt_filter_notdicom = OFFalse ;
OFBool opt_stdin = OFFalse ;
OFBool opt_quiet = OFTrue ;

int badCount = 0;

OFBool isFileDicom(const char* fname)
{
  OFBool ok = OFFalse;
  FILE* f = NULL;

  if (OFStandard::fileExists(fname)) {
    f = fopen(fname, "rb");
    if (f == 0) {
      ok = OFFalse;
    } else {
      char signature[4];
      if ((fseek(f, DCM_PreambleLen, SEEK_SET) < 0) ||
	  (fread(signature, 1, DCM_MagicLen, f) != DCM_MagicLen)) {
	ok = OFFalse;
      } else if (strncmp(signature, DCM_Magic, DCM_MagicLen) != 0) {
	ok = OFFalse;
      } else {
	/* looks ok */
	ok = OFTrue;
      }
      fclose(f);
    }
  }
  
  return ok;
}

void processFile(const std::string fname)
{
  if (isFileDicom(fname.c_str())) {
    if (opt_filter_dicom) {
      COUT << fname << OFendl;
    } else if (!opt_quiet) {
      COUT << "yes: " << fname << OFendl;
    }
  } else {
    badCount = 1;
    if (opt_filter_notdicom) {
      if (opt_filter_dicom) {
	CERR << fname << OFendl;
      } else {
	COUT << fname << OFendl;
      }
    } else if (!opt_quiet) {
      COUT << "no: " << fname << OFendl;
    }
  }
}

int main(int argc, char *argv[])
{
    OFConsoleApplication app(OFFIS_CONSOLE_APPLICATION , "Test if file uses DICOM part 10 format", rcsid);
    OFCommandLine cmd;

    /* evaluate command line */
    cmd.addSubGroup("tweaks:");
    cmd.addOption("--invert-match", "-v", "list non-DICOM input");
    cmd.addOption("--split", "-t", "list DICOM on stdout, non-DICOM on stderr");

    cmd.addParam("file", OFCmdParam::PM_MultiOptional);
    app.parseCommandLine(cmd, argc, argv);

    if (isatty(STDIN_FILENO))
      opt_stdin = OFFalse;
    else
      opt_stdin = OFTrue;

    if (cmd.findOption("--invert-match")) {
      opt_filter_dicom = OFFalse;
      opt_filter_notdicom = OFTrue;
    }

    if (cmd.findOption("--split")) {
      opt_filter_dicom = OFTrue;
      opt_filter_notdicom = OFTrue;
    }


    int count = cmd.getParamCount();
    if (count==0 && !opt_stdin)
      app.printUsage();
    const char *fname = NULL;
    for (int i=1; i<=count; i++) {
      cmd.getParam(i, fname);
      processFile(fname);
    }

    if (opt_stdin)
      {
	std::string fname;
	while(std::cin) {
	  getline(std::cin, fname);
	  if (fname.length() > 0)
	    processFile(fname);
	}
      }

    /*
    ** if all files are ok then return zero, otherwise the
    ** number of non-dicom files.
    */
    return (opt_check) ? badCount : 0;
}
