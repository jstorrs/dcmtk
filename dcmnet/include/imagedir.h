class ImageDirManager
{

private:

  int serialCounter = 0;
  OFString targetAETitle = "";
  char imageFileName[256] = "";
  char tmpFileName[MAXPATHLEN] = "";
  char newFileName[MAXPATHLEN] = "";
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
  }
  
  void
  generateFileNames(const T_DIMSE_C_StoreRQ *req) {
    // [Called AETitle].[YYYYMMDDHHMMSSMMM].[PID].[COUNTER].MODALITY.dcm
    OFDateTime dateTime;
    dateTime.setCurrentDateTime();
    snprintf(imageFileName, 256, "%s.%04u%02u%02u.%02u%02u%02u.%d.%06d.%s.dcm",
	    targetAETitle.c_str(),
	    dateTime.getDate().getYear(), dateTime.getDate().getMonth(), dateTime.getDate().getDay(),
	    dateTime.getTime().getHour(), dateTime.getTime().getMinute(),dateTime.getTime().getIntSecond(),
	    getpid(), serialCounter++, dcmSOPClassUIDToModality(req->AffectedSOPClassUID, "UNKNOWN"));
    snprintf(tmpFileName, MAXPATHLEN, "%s%c%s%c%s", root.c_str(), PATH_SEPARATOR, "tmp", PATH_SEPARATOR, imageFileName);
    snprintf(newFileName, MAXPATHLEN, "%s%c%s%c%s", root.c_str(), PATH_SEPARATOR, "new", PATH_SEPARATOR, imageFileName);
  }

  void
  getTempFileName(char *dst) {
    OFStandard::strlcpy(dst, newFileName, MAXPATHLEN);
  }

  void
  finalizeDelivery() {
    rename(tmpFileName,newFileName);
  }

};
