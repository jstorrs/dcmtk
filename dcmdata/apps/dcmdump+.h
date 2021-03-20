#include "dcmtk/dcmdata/tweaks.h"

OFBool opt_tweak = OFTrue;

namespace Tweak {

  OFBool opt_skip_empty = OFTrue;
  OFBool opt_skip_known_uid = OFFalse;

  void
  addOptions(OFCommandLine& cmd)
  {
    cmd.addGroup("tweaks:");
      cmd.addSubGroup("output:");
        cmd.addOption("--tweak-print",          "-Tp", "print elements using tweaked format");
        cmd.addOption("--tweak-skip-all",       "-Ta", "apply all skip rules");
        cmd.addOption("--tweak-skip-empty",     "-Te", "don't print empty elements");
        cmd.addOption("--tweak-skip-known-uid", "-Tu", "don't print defined UIDs");
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
  DumpPath(const DcmStack& stack,
	   STD_NAMESPACE ostream &out)
  {
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
  }

  
  void
  DumpObject(const DcmStack& stack,
	     STD_NAMESPACE ostream &out,
	     const size_t flags)
  {
    DcmObject* obj = stack.top();
    if (obj->ident() == DcmEVR::EVR_item)
      return;
    if (opt_skip_empty && (obj->getVM() == 0))
      return;
    if (obj->ident() == DcmEVR::EVR_PixelData)
      return;
    if (opt_skip_known_uid && isKnownUID(obj))
      return;
    
    DcmTag tag = obj->getTag();
    DcmVR vr;
    vr.setVR(obj->ident());
    const char* authority = tag.getPrivateCreator();
    authority = authority ? authority : "DICOM";
    out << authority << '\t';
    DumpPath(stack,out);
    out << '\t'
	<< vr.getVRName() << '\t'
	<< obj->getVM() << '\t'
	<< tag.getTagName() << '\t';
    obj->print(out, flags);
    out << OFendl;
  }


  void
  DumpChildren(DcmObject* obj,
	       STD_NAMESPACE ostream &out,
	       const size_t flags)
  {
    DcmStack stack;
    while (obj->nextObject(stack, OFTrue).good()) {
      DumpObject(stack, out, flags);
    }
  }
  
  
  void
  DumpDataset(DcmObject* dset,
	      STD_NAMESPACE ostream &out,
	      const size_t flags = 0)
  {
    Tweaks::PF_ValueOnly = OFTrue;
    DcmFileFormat* dfile = static_cast<DcmFileFormat*>(dset);
    DumpChildren(dfile->getMetaInfo(), out, flags);
    DumpChildren(dfile->getDataset(), out, flags);
  }

  
}

