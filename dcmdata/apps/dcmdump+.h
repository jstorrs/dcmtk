#include "dcmtk/dcmdata/tweaks.h"

namespace Tweak {

  OFBool opt_stdin = OFFalse;
  OFBool opt_print = OFTrue;
  OFBool opt_print_empty = OFFalse;
  OFBool opt_print_filenames = OFFalse;
  OFBool opt_print_known_uid = OFFalse;
  OFBool opt_print_tag_heirarchy = OFTrue;
  char field_sep[] = "\t";
  
  void
  addOptions(OFCommandLine& cmd)
  {
    cmd.addGroup("tweaks:");
      cmd.addSubGroup("output:");
        cmd.addOption("--print-with-filename", "-H", "print filenames");
        cmd.addOption("--print-empty",         "-e", "print empty elements");
	cmd.addOption("--print-known-uid",     "-k", "print known UIDs");
	cmd.addOption("--print-all",           "-a", "print empty elements and known UIDs");
  }


  void
  configureFromCommandLine(OFCommandLine& cmd, OFConsoleApplication& app)
  {
    if (cmd.findOption("--print-with-filename"))
      opt_print_filenames = OFTrue;
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
    const char *symbol = NULL;

    if (obj->ident() == DcmEVR::EVR_UI)
      {
	static_cast<DcmUniqueIdentifier*>(obj)->getString(stringVal,stringLen);
	if ((stringVal != NULL) && (stringLen > 0))
	  symbol = dcmFindNameOfUID(stringVal);
      }
    return ((symbol != NULL) && (strlen(symbol) > 0));
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

  
  void
  DumpObject(const DcmStack& stack,
	     const OFFilename &ifname,
	     STD_NAMESPACE ostream &out,
	     const size_t flags)
  {
    DcmObject* obj = stack.top();
    switch (obj->ident())
      {
      case EVR_metainfo:
      case EVR_dataset:
      case EVR_item:
      case EVR_dirRecord:
	return;
      case EVR_PixelData:
	return;
      case EVR_UI:
	if (!opt_print_known_uid && isKnownUID(obj))
	  return;
      }
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
	<< obj->getVM() << field_sep
	<< tag.getTagName() << field_sep;
    obj->print(out, flags);
    out << OFendl;
  }


  void
  DumpChildren(DcmObject* obj,
	       const OFFilename &ifname,
	       STD_NAMESPACE ostream &out,
	       const size_t flags)
  {
    DcmStack stack;
    while (obj->nextObject(stack, OFTrue).good()) {
      DumpObject(stack, ifname, out, flags);
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
    DumpChildren(dfile->getMetaInfo(), ifname, out, flags);
    DumpChildren(dfile->getDataset(), ifname, out, flags);
  }

  
}

