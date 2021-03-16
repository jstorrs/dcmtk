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
