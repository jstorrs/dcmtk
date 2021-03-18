#include "dcmtk/dcmdata/tweaks.h"

OFBool opt_tweak = OFTrue;

void
AddTweakDumpOptions(OFCommandLine& cmd)
{
  cmd.addGroup("Tweaks:");
    cmd.addOption("--tweak", "-T", "tweak");
}

void
tweak_print_object(STD_NAMESPACE ostream &out,
		   DcmObject *obj)
{
  DcmVR vr;
  DcmTag tag = obj->getTag();
  vr.setVR(obj->ident());
  const char* authority = tag.getPrivateCreator();
  authority = authority ? authority : "DICOM";
  out << authority << '\t'
      << tag << '\t'
      << vr.getVRName() << '\t'
      << obj->getVM() << '\t'
      << tag.getTagName() << '\t';
}


class TweakItem : public DcmItem
{
 public:
  void tweak_print(STD_NAMESPACE ostream &out,
		   const size_t flags,
		   const int level,
		   const char *pixelFileName,
		   size_t *pixelCounter)
  {
    Tweaks::PF_ValueOnly = OFTrue;
    if (!elementList->empty())
      {
        DcmObject *dO;
        elementList->seek(ELP_first);
        do {
	  dO = elementList->get();
	  tweak_print_object(out,dO);
	  dO->print(out, flags, level + 1, pixelFileName, pixelCounter);
	  out << OFendl;
        } while (elementList->seek(ELP_next));
      }
  }
};

#define TweakItem_(x) static_cast<TweakItem*>(static_cast<DcmItem*>(x))

void
TweakDumpDataset(STD_NAMESPACE ostream &out,
		 DcmObject* dset,
		 const size_t flags = 0,
		 const int level = 0,
		 const char *pixelFileName = NULL,
		 size_t *pixelCounter = NULL)
{
  DcmFileFormat* dfile = static_cast<DcmFileFormat*>(dset);
  TweakItem* meta = TweakItem_(dfile->getMetaInfo());
  TweakItem* data = TweakItem_(dfile->getDataset());

  meta->tweak_print(out,flags,level,pixelFileName,pixelCounter);
  data->tweak_print(out,flags,level,pixelFileName,pixelCounter);
}

#undef TweakItem_
