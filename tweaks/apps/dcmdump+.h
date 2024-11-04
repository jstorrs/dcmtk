#include "dcmtk/tweaks/dcmdata.h"

namespace Tweak {

  OFBool opt_stdin = OFFalse;
  OFBool opt_print = OFTrue;
  OFBool opt_print_empty = OFFalse;
  OFBool opt_print_filenames = OFFalse;
  OFBool opt_print_filename_last = OFFalse;
  OFBool opt_print_known_uid = OFFalse;
  OFBool opt_print_tag_heirarchy = OFTrue;
  OFBool opt_tabulate = OFFalse;
  const char* opt_skip_UID = NULL;
  char field_sep[] = "\t";
  
  void
  addOptions(OFCommandLine& cmd)
  {
    cmd.addGroup("tweaks:");
      cmd.addSubGroup("output:");
        cmd.addOption("--print-with-filename", "-H",    "print filenames");
        cmd.addOption("--print-filename-last", "-He",   "print filenames on end");
        cmd.addOption("--print-empty",         "-e",    "print empty elements");
	cmd.addOption("--print-known-uid",     "-k",    "print known UIDs");
	cmd.addOption("--print-all",           "-a",    "print empty elements and known UIDs");
	cmd.addOption("--skip-uid",            "-U", 1, "[U]ID prefix",
		                                        "don't print UIDs that begin with U");
	cmd.addOption("--tabulate",            "-t",    "tabulate");
  }


  void
  configureFromCommandLine(OFCommandLine& cmd, OFConsoleApplication& app)
  {
    if (cmd.findOption("--print-with-filename"))
      opt_print_filenames = OFTrue;
    if (cmd.findOption("--print-filename-last"))
      opt_print_filename_last = OFTrue;
    if (cmd.findOption("--print-all")) {
      opt_print_empty = OFTrue;
      opt_print_known_uid = OFTrue;
    }
    if (cmd.findOption("--print-empty")) {
      opt_print_empty = OFTrue;
    }
    if (cmd.findOption("--print-known-uid")) {
      opt_print_known_uid = OFTrue;
    }
    if (cmd.findOption("--skip-uid")) {
      cmd.getValue(opt_skip_UID);
    }
    if (cmd.findOption("--tabulate")) {
      opt_tabulate = OFTrue;
    }
    if (isatty(STDIN_FILENO))
      opt_stdin = OFFalse;
    else
      opt_stdin = OFTrue;
  }

  
  OFBool
  isKnownUID(DcmObject *obj)
  {
    char *stringVal = NULL;
    Uint32 stringLen = 0;
    OFBool retval = OFFalse;

    if (obj->ident() == EVR_UI)
      {
	static_cast<DcmUniqueIdentifier*>(obj)->getString(stringVal,stringLen);
	if ((stringVal != NULL) && (stringLen > 0)) {
	  const char *symbol = dcmFindNameOfUID(stringVal);
	  if ((symbol != NULL) && (strlen(symbol) > 0))
	    retval = OFTrue;
	  if ((opt_skip_UID != NULL) && (strncmp(stringVal, opt_skip_UID, strlen(opt_skip_UID)) == 0))
	    retval = OFTrue;
	}
      }
    return retval;
  }


  void
  DumpTagPath(const DcmStack& stack,
	      STD_NAMESPACE ostream &out)
  {
    if (opt_print_tag_heirarchy)
      for (int i = stack.card()-1; i>=0; i--)
	{
	  DcmObject* obj = stack.elem(i);
	  switch (obj->ident())
	    {
	    case EVR_metainfo:
	    case EVR_dataset:
	    case EVR_item:
	    case EVR_dirRecord:
	      break;
	    default:
	      out << obj->getTag();
	    }
	}
    else
      out << stack.top()->getTag();
  }

  
  DcmObject*
  GetObjectFromStack(const DcmStack& stack)
  {
    DcmObject* obj = stack.top();
    switch (obj->ident())
      {
      case EVR_metainfo:
      case EVR_dataset:
      case EVR_item:
      case EVR_dirRecord:
      case EVR_PixelData:
	return NULL;
      default:
	break;
      }
    return obj;  
  }


  void
  SafePrint(DcmObject* obj,
	    STD_NAMESPACE ostream &out,
	    const size_t flags)
  {
    if (obj) {
      std::stringstream ss;
      Tweaks::PF_ValueOnly = OFTrue;    
      obj->print(ss, flags);
      std::string s = ss.str();
      if (opt_tabulate) {
	if ((s.front() == '[') & (s.back() == ']'))
	  s = s.substr(1, s.size()-2);
      }
      out << s;
    }
  }
  
  
  void
  PrintRow(const DcmStack& stack,
	   DcmObject* obj,
	   const OFFilename &ifname,
	   STD_NAMESPACE ostream &out,
	   const size_t flags)
  {
    if (obj) {
      if (!opt_print_known_uid && isKnownUID(obj))
	return;
      
      if (!opt_print_empty && (obj->getVM() == 0))
	return;

      DcmTag tag = obj->getTag();
      DcmVR vr;
      vr.setVR(obj->ident());
      const char* authority = tag.getPrivateCreator();
      authority = authority ? authority : "DICOM";
      if (opt_print_filenames)
	out << ifname << field_sep;
      out << authority << field_sep;
      DumpTagPath(stack,out);
      out << field_sep
	  << vr.getVRName() << field_sep
	  << tag.getTagName() << field_sep
	  << obj->getVM() << field_sep;
      SafePrint(obj, out, flags);
      out << OFendl;
    }
  }

  
  OFBool first_tag = OFFalse;
  
  void
  FileBegin(const OFFilename &ifname,
	    STD_NAMESPACE ostream &out)
  {
    first_tag = OFTrue;
    if (opt_tabulate & opt_print_filenames)
      out << ifname << field_sep;
  }


  void
  TagBegin(STD_NAMESPACE ostream &out)
  {
    if (opt_tabulate & !first_tag)
      out << field_sep;
    first_tag = OFFalse;
  }


  void
  TagEnd(STD_NAMESPACE ostream &out)
  {
    ;
  }


  void
  FileEnd(const OFFilename &ifname,
	  STD_NAMESPACE ostream &out)
  {
    if (opt_tabulate) {
      if (opt_print_filename_last)
	out << field_sep << ifname << OFendl;
      else
	out << OFendl;
    }
  }


  void
  DumpObject(const DcmStack& stack,
	     const OFFilename &ifname,
	     STD_NAMESPACE ostream &out,
	     const size_t flags)
  {
    Tweaks::PF_ValueOnly = OFTrue;
    DcmObject* obj = GetObjectFromStack(stack);
    if (opt_tabulate)
      SafePrint(obj, out, flags);
    else
      PrintRow(stack, obj, ifname, out, flags);
  }
  

  void
  DumpChildren(DcmObject* obj,
	       const OFFilename &ifname,
	       STD_NAMESPACE ostream &out,
	       const size_t flags)
  {
    DcmStack stack;
    while (obj->nextObject(stack, OFTrue).good()) {
      TagBegin(out);
      DumpObject(stack, ifname, out, flags);
      TagEnd(out);
    }
  }
  
  
  void
  DumpDataset(DcmObject* dset,
	      const OFFilename &ifname,
	      STD_NAMESPACE ostream &out,
	      const size_t flags = 0)
  {
    Tweaks::PF_ValueOnly = OFTrue;
    DcmFileFormat* dfile = static_cast<DcmFileFormat*>(dset);
    FileBegin(ifname,out);
    DumpChildren(dfile->getMetaInfo(), ifname, out, flags);
    DumpChildren(dfile->getDataset(), ifname, out, flags);
    FileEnd(ifname,out);
  }

  
}

