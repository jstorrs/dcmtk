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
  is_known_uid(DcmObject *obj)
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

  // Forward Declaration
  void DumpChildren(DcmObject* obj, STD_NAMESPACE ostream &out, const size_t flags);

  
  void
  DumpObject(DcmObject* obj,
	     STD_NAMESPACE ostream &out,
	     const size_t flags)
  {
    DumpChildren(obj, out, flags);

    if (opt_skip_empty && (obj->getVM() == 0))
      return;
    if (obj->ident() == DcmEVR::EVR_PixelData)
      return;
    // if (is_known_uid(obj))
    //   return;
  
    DcmTag tag = obj->getTag();
    DcmVR vr;
    vr.setVR(obj->ident());
    const char* authority = tag.getPrivateCreator();
    authority = authority ? authority : "DICOM";
    out << authority << '\t'
	<< tag << '\t'
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
    DcmList* list = obj->getChildren();
    if (list && !list->empty())
      {
	DcmObject *obj;
	list->seek(ELP_first);
	do {
	  obj = list->get();
	  DumpObject(obj, out, flags);
	} while (list->seek(ELP_next));
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

