class ImageDirManager
{

private:

  int serialCounter = 0;
  OFString targetAETitle = "";
  OFString tmpFileName = "";
  OFString newFileName = "";
  OFString tmpDir = "";
  OFString newDir = "";
  OFString root = "";
  
public:

  OFBool active = OFFalse;

  void
  addOptionGroup(OFCommandLine &cmd)
  {
    cmd.addGroup("tweaks:");
      cmd.addSubGroup("imagedir output:");
        cmd.addOption("--imagedir", "enable imagedir output mode");
  }

  void
  parseOptions(OFConsoleApplication& app, OFCommandLine& cmd)
  {
    active = OFFalse;

    cmd.beginOptionBlock();
    if (cmd.findOption("--imagedir"))
      {
#define IMAGEDIR_CHECK_CONFLICT(x) if (cmd.findOption(x)) app.checkConflict("--imagedir",x,OFTrue)
	IMAGEDIR_CHECK_CONFLICT("--timenames");
	IMAGEDIR_CHECK_CONFLICT("--sort-conc-studies");
	IMAGEDIR_CHECK_CONFLICT("--sort-on-study-uid");
	IMAGEDIR_CHECK_CONFLICT("--sort-on-patientname");
	IMAGEDIR_CHECK_CONFLICT("--exec-on-reception");
	IMAGEDIR_CHECK_CONFLICT("--exec-on-eostudy");
	IMAGEDIR_CHECK_CONFLICT("--rename-on-eostudy");
	IMAGEDIR_CHECK_CONFLICT("--eostudy-timeout");
	IMAGEDIR_CHECK_CONFLICT("--exec-sync");
#undef IMAGEDIR_CHECK_CONFLICT
	active = OFTrue;
      }
    cmd.endOptionBlock();
  }

  void
  setOutputDirectory(const OFString &outputDirectory)
  {
    root = outputDirectory;
  }
  
  void
  setCalledAETitle(const DIC_AE calledTitle)
  {
    OFString tempAETitle = OFSTRING_GUARD(calledTitle);
    OFString::iterator it, itA, itZ;
    targetAETitle.clear();
    itA = tempAETitle.begin();
    itZ = tempAETitle.end();
    for ( ; itA < itZ; itA++)
      if (isgraph(*itA)) break ;
    for ( ; itA < itZ; itZ--)
      if (isgraph(*itZ)) break ;
    for (it = itA; it <= itZ; it++) {
      if (isalnum(*it) || (*it == '-') || (*it == '.')) {
	targetAETitle += *it;
      } else {
	targetAETitle += "_";
      }
    }
    OFStandard::combineDirAndFilename(tmpDir,root,"tmp");
    OFStandard::combineDirAndFilename(newDir,root,targetAETitle);
    if (!isValidDestination(newDir))
      OFStandard::combineDirAndFilename(newDir,root,"new");
  }
  
  void
  generateFileNames(const T_DIMSE_C_StoreRQ *req) {
    // [Called AETitle].[YYYYMMDDHHMMSSMMM].[PID].[COUNTER].MODALITY.dcm
    char imageFileName[NAME_MAX] = "";
    OFDateTime dateTime;
    dateTime.setCurrentDateTime();
    snprintf(imageFileName, NAME_MAX, "%s.%04u%02u%02u%02u%02u%02u%03u.%d.%06d.%s.dcm",
	     targetAETitle.c_str(),
	     dateTime.getDate().getYear(), dateTime.getDate().getMonth(), dateTime.getDate().getDay(),
	     dateTime.getTime().getHour(), dateTime.getTime().getMinute(),dateTime.getTime().getIntSecond(), dateTime.getTime().getMilliSecond(),
	     getpid(), serialCounter++, dcmSOPClassUIDToModality(req->AffectedSOPClassUID, "UNKNOWN"));
    OFStandard::combineDirAndFilename(tmpFileName,tmpDir,imageFileName);
    OFStandard::combineDirAndFilename(newFileName,newDir,imageFileName);
  }

  void
  getTempFileName(char *dst) {
    OFStandard::strlcpy(dst, tmpFileName.c_str(), PATH_MAX);
  }

  void
  finalizeDelivery() {
    OFStandard::renameFile(tmpFileName,newFileName);
  }

  bool
  isValidDestination(const OFString& dir)
  {
    return (OFStandard::dirExists(dir) && OFStandard::isWriteable(dir));
  }

};
